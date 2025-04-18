#pragma once
#include "Types.h"
#include "Containers\GrowingArray\GrowingArray.h"


namespace Hail
{
	struct CloudParticle
	{
		glm::vec2 pos;
		glm::vec2 predictedPos;
		glm::vec2 velocity;
		glm::vec2 densityNearDensity;
	};

	struct RenderCommandPool;

	struct SpatialIndexLookup
	{
		uint32 cellKey;
		uint32 particleIndex;
	};

	class CloudParticleSimulator
	{
	public:

		void UpdateParticles(GrowingArray<CloudParticle>& cloudParticles, RenderCommandPool& poolOfCommands, glm::uvec2 resolution, const GrowingArray<float>& distanceField);

		void UpdatePartialLookupList(GrowingArray<CloudParticle>& cloudParticles);

		glm::vec2 CalculateDensityForEachPointWithinRadius(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck);
		glm::vec2 CalculatePressureForceForEachPointWithinRadius(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck);
		glm::vec2 CalculateViscosityForceForEachPointWithinRadius(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck);
		glm::vec2 CalculateForceToSdf(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck, const GrowingArray<float>& distanceField);
	private:
		float m_particleRadius = 0.11f;
		float m_targetDensity = 2.0f;
		float m_pressureMultiplier = 4.f;
		float m_nearPressureMultiplier = -0.2f;
		float m_viscosityForce = 0.60f;
		
		float m_gravity = -1.f;
		float m_mouseForceStrength = 10.f;
		float m_mouseForceRadius = 0.25f;
		float m_particleSpacing = 0.04f;

		glm::ivec2 m_numberOfPointsToSpawn = glm::ivec2(5, 5);
		GrowingArray<CloudParticle> m_particleGrid;
		GrowingArray<CloudParticle> m_cloudParticles;
		int m_timeStepsPerSecond = 120;

		uint32 m_particleGridSize = 128u;
		uint32 m_numberOfParticles;

		GrowingArray<SpatialIndexLookup> m_particleSpatialLookup;
		GrowingArray<uint32> m_particleSpatialLookupStartIndices;

		bool m_bSimulateGrid = false;
		bool m_bShowCircles = true;
		bool m_bShowGradientValues = true;
		bool m_bUpdatedParticleGrid = false;
	};

}

