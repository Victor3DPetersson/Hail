#include "Engine_PCH.h"
#include "CloudParticleSimulator.h"

#include "HailEngine.h"
#include "Timer.h"
#include "Utility\DebugLineHelpers.h"
#include "RenderCommands.h"

#include "Utility\Sorting.h"
#include "Input\InputHandler.h"


#include "Containers\StaticArray\StaticArray.h"

#include "imgui.h"

namespace Hail
{
	float g_globalRadius = 0.f;
	float g_targetDensity = 0.f;
	float g_targetBaseDensity = 0.f;
	float g_pressureMultiplier = 0.f;
	float g_nearPressureMultiplier = 0.f;

	glm::vec2 spaceModifier = glm::vec2(10.f);

	void CollideWithBounds(CloudParticle& particleToCheck, float particleRadius)
	{
		float halfRadius = particleRadius * 0.5f;
		const float collisionDampening = 0.1f;
		glm::vec2 upperBounds = spaceModifier - halfRadius;
		glm::vec2 lowerBounds = glm::vec2(halfRadius);

		if (Math::Abs(particleToCheck.pos.x) > upperBounds.x)
		{
			particleToCheck.pos.x = upperBounds.x;
			particleToCheck.velocity.x *= -1.f * collisionDampening;
		}
		if (Math::Abs(particleToCheck.pos.y) > upperBounds.y)
		{
			particleToCheck.pos.y = upperBounds.y;
			particleToCheck.velocity.y *= -1.f * collisionDampening;
		}

		if (Math::Abs(particleToCheck.pos.x) < lowerBounds.x)
		{
			particleToCheck.pos.x = lowerBounds.x;
			particleToCheck.velocity.x *= -1.f * collisionDampening;
		}
		if (Math::Abs(particleToCheck.pos.y) < lowerBounds.y)
		{
			particleToCheck.pos.y = lowerBounds.y;
			particleToCheck.velocity.y *= -1.f * collisionDampening;
		}
	}

	float ViscosityKernel(float radius, float distance)
	{
		if (distance >= radius) return 0.f;
		// function (r^2-d^2)^3
		// volume over the entire radius in a circle
		float volume = (Math::PIf * powf(radius, 8.f)) / 4.f;
		float f = radius * radius - distance * distance;
		return f * f * f / volume;
	}

	float SmoothingKernel(float radius, float distance)
	{
		if (distance >= radius) return 0.f;
		// Volume for (r-d)^3
		//float volume = (Math::PIf * powf(radius, 5.f)) * 0.1f;
		//float value = radius - distance;
		//return value * value * value / volume;

		// Volume for (r-d)^2
		float volume = (Math::PIf * powf(radius, 4.f)) / 6.f;
		float value = radius - distance;
		return value * value / volume;
	}
	float NearSmoothingKernel(float radius, float distance)
	{
		if (distance >= radius) return 0.f;
		// Volume for (r-d)^3
		float volume = (Math::PIf * powf(radius, 5.f)) * 0.1f;
		float value = radius - distance;
		return value * value * value / volume;
	}

	float SmoothingKernelDerivative(float radius, float distance)
	{
		if (distance >= radius) return 0.f;
		// Derivative for (r-d)^3
		//float f = radius - distance;
		//float scale = (-30.f * (f * f)) / (Math::PIf * powf(radius, 5.f));

		// Derivative for (r-d)^2
		float f = radius - distance;
		float scale = (-12.f * f) / (Math::PIf * powf(radius, 4.f));

		return scale;
	}

	glm::vec2 CalculateDensity(glm::vec2 pointToCheck, float radiusToCheck, const GrowingArray<CloudParticle>& particlesToCheckForDensity)
	{
		float calculatedDensity = 0.f;
		float calculatedNearDensity = 0.f;
		const float mass = 1.f;

		for (uint32 i = 0; i < particlesToCheckForDensity.Size(); i++)
		{
			float distance = glm::length(pointToCheck - particlesToCheckForDensity[i].pos);
			float influence = SmoothingKernel(radiusToCheck, distance);
			float nearInfluence = NearSmoothingKernel(radiusToCheck, distance);
			calculatedDensity += mass * influence;
			calculatedNearDensity += mass * nearInfluence;
		}

		return glm::vec2(calculatedDensity, calculatedNearDensity);
	}

	glm::vec2 ConvertDensityToPressure(glm::vec2 densityNearDensity)
	{
		float densityError = densityNearDensity.x - g_targetDensity;
		float density = densityError * g_pressureMultiplier;
		float nearDensity = densityNearDensity.y * g_nearPressureMultiplier;
		return glm::vec2(density, nearDensity);
	}

	glm::vec2 VogelDirection(uint32 sampleIndex, uint32 samplesCount, float Offset)
	{
		float r = sqrt(float(sampleIndex) + 0.5f) / sqrt(float(samplesCount));
		float theta = float(sampleIndex) * Math::GoldenAngle + Offset;
		return r * glm::vec2(cos(theta), sin(theta));
	}

	glm::vec2 CalculateSharedPressure(glm::vec2 densityA, glm::vec2 densityB)
	{
		glm::vec2 pressureA = ConvertDensityToPressure(densityA);
		glm::vec2 pressureB = ConvertDensityToPressure(densityB);
		return (pressureA + pressureB) * 0.5f;
	}

	const int32 HASH_SIZE = 200;
	const float L = 0.2f;

	inline uint32 hasha(glm::ivec2& p) {
		int ix = (unsigned int)((p[0] + 2.f) / L);
		int iy = (unsigned int)((p[1] + 2.f) / L);
		return (unsigned int)((ix * 73856093) ^ (iy * 19349663)) % HASH_SIZE;
	}

	glm::ivec2 PositionToCellCoord(glm::vec2 positionToCheck)
	{
		glm::vec2 worldCenter = glm::vec2(0.5f) * spaceModifier;
		glm::vec2 adjustedCenter = positionToCheck - worldCenter;

		return adjustedCenter / g_globalRadius;
	}

	uint32 GenerateKeyFromCellPos(glm::ivec2 cellCoord, uint32 listMaxLength)
	{
		return hasha(cellCoord) % listMaxLength;
	}

	uint32 ComputeParticleKeyLookup(const CloudParticle& cloudParticle, uint32 listMaxLength)
	{
		glm::ivec2 cellCoord = PositionToCellCoord(cloudParticle.pos);
		return GenerateKeyFromCellPos(cellCoord, listMaxLength);
	}

	glm::ivec2 ParticlePosToSdfCoords(glm::vec2 particlePosition)
	{
		glm::vec2 normalizedParticlePos = particlePosition / spaceModifier;
		return glm::ivec2(normalizedParticlePos.x * 128, normalizedParticlePos.y * 128);
	}

	glm::vec2 MouseForce(glm::uvec2 renderResolution,  float radius, float mouseForceStrength, const CloudParticle& particleToCheck)
	{
		glm::vec2 mouseForce = glm::vec2(0.f);

		glm::vec2 mouseParticleSpacePos = (glm::vec2(GetInputHandler().GetInputMap().mouse.mousePos) / (float)renderResolution.y) * spaceModifier;

		glm::vec2 offset = particleToCheck.pos - mouseParticleSpacePos;
		float sqrDistance = glm::dot(offset, offset);

		const bool lmbDown = GetInputHandler().GetInputMap().mouse.keys[(uint32)eMouseMapping::LMB] == (uint8)eInputState::Down;
		const bool rmbDown = GetInputHandler().GetInputMap().mouse.keys[(uint32)eMouseMapping::RMB] == (uint8)eInputState::Down;

		float strength = 0.f;
		if (lmbDown)
		{
			strength = 1.f;
		}
		else if (rmbDown)
		{
			strength = -1.f;
		}

		if (strength != 0.f && sqrDistance < radius * radius)
		{
			float distance = glm::length(offset);

			// Normalized or Zero
			glm::vec2 dirToMouse = distance <= FLT_EPSILON ? glm::vec2(0.f) : offset / distance;

			float centreT = 1.f - distance / radius;
			mouseForce += (dirToMouse * strength - particleToCheck.velocity) * centreT;
		}
		return mouseForce * mouseForceStrength;
	}

	void CloudParticleSimulator::UpdateParticles(GrowingArray<CloudParticle>& cloudParticles, RenderCommandPool& poolOfCommands, glm::uvec2 resolution, const GrowingArray<float>& distanceField)
	{
		const float dt = GetRenderLoopTimer().GetDeltaTime();
		if (dt > 1.0)
			return;

		bool respawnGrid = false;
		if (!m_bUpdatedParticleGrid)
		{
			respawnGrid = true;
			m_cloudParticles.PrepareAndFill(cloudParticles.Size());
			for (uint32 i = 0; i < cloudParticles.Size(); i++)
			{
				m_cloudParticles[i] = cloudParticles[i];
				m_cloudParticles[i].pos *= spaceModifier;
			}
			m_bUpdatedParticleGrid = true;
		}


		ImGui::Begin("Particle test window");

		if (ImGui::SliderFloat("Particle radius", &m_particleRadius, 0.f, 1.f))
		{
			respawnGrid = true;
		}
		g_globalRadius = m_particleRadius * spaceModifier.x;

		ImGui::SliderFloat("Target Density", &m_targetDensity, -100.f, 500.f);
		g_targetBaseDensity = SmoothingKernel(g_globalRadius, 0.f);
		g_targetDensity = m_targetDensity + g_targetBaseDensity;
		ImGui::SliderFloat("Pressure Multiplier", &m_pressureMultiplier, 0.f, 100.f);
		g_pressureMultiplier = m_pressureMultiplier;
		ImGui::SliderFloat("Near Pressure Multiplier", &m_nearPressureMultiplier, 0.f, 10.f);
		g_nearPressureMultiplier = m_nearPressureMultiplier;
		ImGui::SliderFloat("Viscosity force", &m_viscosityForce, 0.f, 10.f);
		ImGui::SliderFloat("Mouse force", &m_mouseForceStrength, 0.f, 100.f);
		ImGui::SliderFloat("Mouse radius", &m_mouseForceRadius, 0.f, 1.f);
		if (ImGui::SliderInt2("NumberOfPoints", &m_numberOfPointsToSpawn.x, 1, 100))
		{
			respawnGrid = true;
		}
		if (ImGui::SliderFloat("Grid Spacing", &m_particleSpacing, -1.f, 1.f))
		{
			respawnGrid = true;
		}
		ImGui::SliderFloat("Gravity", &m_gravity, -1.f, 1.f);
		ImGui::SliderInt("Time Steps per second", &m_timeStepsPerSecond, 30, 600);
		// Settings
		if (ImGui::Checkbox("Simulate Grid", &m_bSimulateGrid))
		{
			respawnGrid = true;
		}
		ImGui::Checkbox("Draw Circles", &m_bShowCircles);
		ImGui::Checkbox("Draw Gradient Vectors", &m_bShowGradientValues);

		if (respawnGrid)
		{
			m_particleGrid.RemoveAll();
			m_numberOfParticles = m_numberOfPointsToSpawn.x * m_numberOfPointsToSpawn.y;
			m_particleGridSize = Math::Max(m_particleGridSize, m_numberOfParticles);
			m_particleGrid.PrepareAndFill(m_particleGridSize);

			float spacing = g_globalRadius * 2 + m_particleSpacing * spaceModifier.x;
			float rowLength = (1.f / (float)m_numberOfPointsToSpawn.x) * 0.5f * spaceModifier.x;
			float columnLength = (1.f / (float)m_numberOfPointsToSpawn.y) * 0.5f * spaceModifier.y;

			//glm::vec2 minSpace = glm::vec2(spaceModifier.x / 2 - rowLength / 2, spaceModifier.y / 2 - columnLength / 2);
			glm::vec2 minSpace = glm::vec2(spaceModifier.x / 2, spaceModifier.y / 2) - glm::vec2((float)m_numberOfPointsToSpawn.x * spacing / 2 , (float)m_numberOfPointsToSpawn.y * spacing / 2);

			uint32 numberOfPointsToSpawn = m_numberOfPointsToSpawn.x * m_numberOfPointsToSpawn.y;
			for (uint32 i = 0; i < numberOfPointsToSpawn; i++)
			{
				CloudParticle particle;

				uint32 xCoord = i % m_numberOfPointsToSpawn.x;
				uint32 yCoord = i / m_numberOfPointsToSpawn.x;
				particle.velocity = glm::vec2(0.f);
				float x = minSpace.x + xCoord * spacing;
				float y = minSpace.y + yCoord * spacing;
				particle.pos = glm::vec2(x, y);
				m_particleGrid[i] = particle;
			}
		}

		float horizontalAspectRatio = float(resolution.x) / float(resolution.y);
		float inverseHorizontalAspectRatio = 1.f / horizontalAspectRatio;

		if (!m_bSimulateGrid)
		{
			m_numberOfParticles = cloudParticles.Size();
			m_particleGridSize = cloudParticles.Capacity();
		}

		GrowingArray<CloudParticle>& particlesToSimulate = m_bSimulateGrid ? m_particleGrid : m_cloudParticles;

		// Draw density debug circle
		{
			glm::vec2 mouseNormalizedPos = glm::vec2(GetInputHandler().GetInputMap().mouse.mousePos) / (float)resolution.y;
			glm::vec2 adjustedDebugPos = glm::vec2(mouseNormalizedPos.x * inverseHorizontalAspectRatio, mouseNormalizedPos.y);
			DrawCircle2D(poolOfCommands.m_debugLineCommands, inverseHorizontalAspectRatio, adjustedDebugPos, m_mouseForceRadius);
			// Fetch density at a debug point
			glm::vec2 densityAtPoint = CalculateDensity(mouseNormalizedPos * spaceModifier, g_globalRadius, particlesToSimulate) - g_targetBaseDensity;
			ImGui::Text("Density at point %f, near Density at point %f", densityAtPoint.x, densityAtPoint.y);
		}

		float gravity = m_gravity;
		const float lookAheadStepRate = 1.f / (float)m_timeStepsPerSecond;

		for (uint32 i = 0; i < m_numberOfParticles; i++)
		{
			glm::vec2 graviticForce = m_bSimulateGrid ? glm::vec2(0.f, 1.f) : CalculateForceToSdf(particlesToSimulate, i, distanceField);
			//glm::vec2 graviticForce = glm::vec2(0.f, 1.f);

			particlesToSimulate[i].velocity += graviticForce * gravity * dt;
			particlesToSimulate[i].predictedPos = particlesToSimulate[i].pos + particlesToSimulate[i].velocity * lookAheadStepRate;
		}

		UpdatePartialLookupList(particlesToSimulate);

		for (uint32 i = 0; i < m_numberOfParticles; i++)
		{
			particlesToSimulate[i].densityNearDensity = CalculateDensityForEachPointWithinRadius(particlesToSimulate, i);
			if (particlesToSimulate[i].densityNearDensity.x == 0.f)
			{
				particlesToSimulate[i].densityNearDensity.x = 0.f;
			}

		}

		for (uint32 i = 0; i < m_numberOfParticles; i++)
		{
			glm::vec2 pressureForce = CalculatePressureForceForEachPointWithinRadius(particlesToSimulate, i);
			glm::vec2 viscosityForce = CalculateViscosityForceForEachPointWithinRadius(particlesToSimulate, i) * m_viscosityForce;
			glm::vec2 mouseForce = MouseForce(resolution, m_mouseForceRadius * spaceModifier.x, m_mouseForceStrength, particlesToSimulate[i]);
			//glm::vec2 pressureForce = CalculatePressureForce(i, particlesToSimulate);
			float density = particlesToSimulate[i].densityNearDensity.x;
			glm::vec2 pressureAcceleration = (pressureForce + viscosityForce) / density + mouseForce;
			particlesToSimulate[i].velocity += pressureAcceleration * dt;
		}

		for (uint32 i = 0; i < m_numberOfParticles; i++)
		{
			particlesToSimulate[i].pos += particlesToSimulate[i].velocity * dt;
			CollideWithBounds(particlesToSimulate[i], g_globalRadius);
		}

		ImGui::End();

		DebugCircle debugCircle;
		debugCircle.scale = (m_particleRadius * (float)resolution.y);
		debugCircle.color = { 3.0 / 255.f, 169.f / 255.f, 252.f / 255.f };
		for (uint32 i = 0; i < m_numberOfParticles; i++)
		{
			debugCircle.pos = particlesToSimulate[i].pos / spaceModifier;
			debugCircle.pos.x *= inverseHorizontalAspectRatio;
			if (m_bShowCircles)
				poolOfCommands.m_debugCircles.Add(debugCircle);

			if (m_bShowGradientValues)
			{
				glm::vec2 adjustedVelocity = glm::vec2(particlesToSimulate[i].velocity.x * inverseHorizontalAspectRatio, particlesToSimulate[i].velocity.y) / spaceModifier;
				glm::vec2 endPos = adjustedVelocity + debugCircle.pos;
				DrawLine2D(poolOfCommands.m_debugLineCommands, debugCircle.pos, endPos, Color::Red, Color::LimeGreen);
			}
		}
		debugCircle.scale = debugCircle.scale * 0.1f ;
		debugCircle.color = Color::Black;
		for (uint32 i = 0; i < m_numberOfParticles; i++)
		{
			debugCircle.pos = particlesToSimulate[i].pos / spaceModifier;
			debugCircle.pos.x *= inverseHorizontalAspectRatio;
			if (m_bShowCircles)
				poolOfCommands.m_debugCircles.Add(debugCircle);
		}

		if (!m_bSimulateGrid)
		{
			for (uint32 i = 0; i < cloudParticles.Size(); i++)
			{
				cloudParticles[i] = m_cloudParticles[i];
				cloudParticles[i].pos /= spaceModifier;
			}
		}
	}

	StaticArray<glm::ivec2, 9> cellOffsets = {
		{-1, -1},
		{0, -1},
		{1, -1},
		{-1, 0},
		{0, 0},
		{1, 0},
		{-1, 1},
		{0, 1},
		{1, 1}
	};

	void CloudParticleSimulator::UpdatePartialLookupList(GrowingArray<CloudParticle>& cloudParticles)
	{
		const uint32 numberOfParticles = m_numberOfParticles;
		m_particleSpatialLookup.RemoveAll();
		m_particleSpatialLookup.PrepareAndFill(m_particleGridSize);
		m_particleSpatialLookupStartIndices.PrepareAndFill(m_particleGridSize);
		for (uint32 i = 0; i < numberOfParticles; i++)
		{
			uint32 key = ComputeParticleKeyLookup(cloudParticles[i], m_particleGridSize);
			m_particleSpatialLookup[i] = (SpatialIndexLookup{ key, i });
		}
		for (uint32 i = 0; i < m_particleGridSize; i++)
		{
			m_particleSpatialLookupStartIndices[i] = MAX_UINT;
		}

		SpatialIndexLookup* lookupStart = m_particleSpatialLookup.Data();
		Sorting::LinearBubbleParticleLookup(&lookupStart, numberOfParticles);
		
		if (numberOfParticles)
		{
			for (uint32 i = 0u; i < numberOfParticles; i++)
			{
				uint32 key = m_particleSpatialLookup[i].cellKey;
				uint32 keyPrevious = i == 0u ? MAX_UINT : m_particleSpatialLookup[i - 1].cellKey;
				if (key != keyPrevious)
				{
					m_particleSpatialLookupStartIndices[key] = (i);
				}
			}
		}
	}

	glm::vec2 CloudParticleSimulator::CalculateDensityForEachPointWithinRadius(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck)
	{
		glm::vec2 calculatedDensity = glm::vec2(0.f);
		const float mass = 1.f;

		const CloudParticle& particle = cloudParticleList[indexToCheck];
		glm::ivec2 cellCoord = PositionToCellCoord(particle.predictedPos);
		float sqrRadius = g_globalRadius * g_globalRadius;

		for (uint32 iCellID = 0; iCellID < 9; iCellID++)
		{
			const glm::ivec2 cellCoordToTest = cellCoord + cellOffsets[iCellID];
			const uint32 key = GenerateKeyFromCellPos(cellCoordToTest, m_particleGridSize);
			const uint32 cellStartIndex = m_particleSpatialLookupStartIndices[key];
			for (uint32 i = cellStartIndex; i < m_numberOfParticles; i++)
			{
				if (m_particleSpatialLookup[i].cellKey != key)
					break;

				const uint32 particleIndex = m_particleSpatialLookup[i].particleIndex;
				glm::vec2 diffVector = particle.predictedPos - cloudParticleList[particleIndex].predictedPos;
				float sqrDist = glm::dot(diffVector, diffVector);

				if (sqrDist <= sqrRadius)
				{
					float distance = glm::length(particle.predictedPos - cloudParticleList[particleIndex].predictedPos);
					float influence = SmoothingKernel(g_globalRadius, distance);
					float nearInfluence = NearSmoothingKernel(g_globalRadius, distance);
					calculatedDensity.x += mass * influence;
					calculatedDensity.y += mass * nearInfluence;
				}
			}
		}
		return calculatedDensity;
	}

	glm::vec2 CloudParticleSimulator::CalculatePressureForceForEachPointWithinRadius(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck)
	{
		glm::vec2 pressureForce = glm::vec2(0.f);
		const float mass = 1.f;

		const CloudParticle& particle = cloudParticleList[indexToCheck];
		glm::ivec2 cellCoord = PositionToCellCoord(particle.predictedPos);
		float sqrRadius = g_globalRadius * g_globalRadius;

		for (uint32 iCellID = 0; iCellID < 9; iCellID++)
		{
			uint32 key = GenerateKeyFromCellPos(cellCoord + cellOffsets[iCellID], m_particleGridSize);
			uint32 cellStartIndex = m_particleSpatialLookupStartIndices[key];
			for (uint32 i = cellStartIndex; i < m_numberOfParticles; i++)
			{
				if (m_particleSpatialLookup[i].cellKey != key)
					break;
				if (m_particleSpatialLookup[i].particleIndex == indexToCheck)
					continue;

				uint32 particleIndex = m_particleSpatialLookup[i].particleIndex;
				glm::vec2 diffVector = particle.predictedPos - cloudParticleList[particleIndex].predictedPos;
				float sqrDist = glm::dot(diffVector, diffVector);

				if (sqrDist <= sqrRadius)
				{
					glm::vec2 direction = particle.predictedPos - cloudParticleList[particleIndex].predictedPos;
					float distance = glm::length(direction);
					// normalize the direction
					direction = distance == 0.f ? VogelDirection(i, 256u + indexToCheck, (indexToCheck + 1u) * 0.01f) : direction / distance;
					float slope = SmoothingKernelDerivative(g_globalRadius, distance);
					glm::vec2 density = cloudParticleList[particleIndex].densityNearDensity;
					glm::vec2 sharedPressure = CalculateSharedPressure(density, particle.densityNearDensity);
					pressureForce += -(sharedPressure.x - sharedPressure.y) * mass * direction * slope / density.x;
				}
			}
		}
		return pressureForce;
	}

	glm::vec2 CloudParticleSimulator::CalculateViscosityForceForEachPointWithinRadius(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck)
	{
		glm::vec2 viscosityForce = glm::vec2(0.f);
		const float mass = 1.f;

		const CloudParticle& particle = cloudParticleList[indexToCheck];
		glm::ivec2 cellCoord = PositionToCellCoord(particle.predictedPos);
		float sqrRadius = g_globalRadius * g_globalRadius;

		for (uint32 iCellID = 0; iCellID < 9; iCellID++)
		{
			uint32 key = GenerateKeyFromCellPos(cellCoord + cellOffsets[iCellID], m_particleGridSize);
			uint32 cellStartIndex = m_particleSpatialLookupStartIndices[key];
			for (uint32 i = cellStartIndex; i < m_numberOfParticles; i++)
			{
				if (m_particleSpatialLookup[i].cellKey != key)
					break;
				if (m_particleSpatialLookup[i].particleIndex == indexToCheck)
					continue;

				uint32 particleIndex = m_particleSpatialLookup[i].particleIndex;
				const CloudParticle& particleToCheckAgainst = cloudParticleList[particleIndex];
				glm::vec2 direction = particle.predictedPos - particleToCheckAgainst.predictedPos;
				float distance = glm::length(direction);
				// normalize the direction
				float influence = ViscosityKernel(g_globalRadius, distance);
				viscosityForce += (particleToCheckAgainst.velocity - particle.velocity) * influence;
			}
		}
		return viscosityForce;
	}

	glm::vec2 CloudParticleSimulator::CalculateForceToSdf(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck, const GrowingArray<float>& distanceField)
	{
		const CloudParticle& particle = cloudParticleList[indexToCheck];

		glm::ivec2 particleSdfPos = ParticlePosToSdfCoords(particle.pos);

		float sdfAtPos = distanceField[particleSdfPos.x + particleSdfPos.y * 128];

		if (sdfAtPos < 0.f)
			return glm::vec2(0.f);


		// calculate the normal to the closest cloud point
		int xShift = 1;
		int yShift = 1;

		if (particleSdfPos.x == 127)
			xShift = -1;
		if (particleSdfPos.y == 127)
			yShift = -1;

		float xSdf = distanceField[(particleSdfPos.x + xShift) + (particleSdfPos.y) * 128];
		float ySdf = distanceField[(particleSdfPos.x) + (particleSdfPos.y + yShift) * 128];

		float xGradient = xSdf - sdfAtPos;
		float yGradient = ySdf - sdfAtPos;

		xGradient = xGradient == 0.f ? (float)xShift : xGradient;
		yGradient = yGradient == 0.f ? (float)yShift : yGradient;

		glm::vec2 returnVec = glm::normalize(glm::vec2(xGradient, yGradient));
		if (std::isnan(returnVec.x) || std::isnan(returnVec.y))
			return glm::vec2(0.f);
		return returnVec;
	}

}