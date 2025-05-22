#pragma once
#include "Types.h"
#include "Containers\GrowingArray\GrowingArray.h"


namespace Hail
{
	struct CloudParticle
	{
		glm::vec2 pos;
		glm::vec2 velocity;
		glm::vec2 intermediateVelocity;
		glm::vec2 pressureForce = glm::vec2(0.f);
		float pressure = 0.f;
		float nearPressure = 0.f;
		glm::vec2 densityNearDensity = glm::vec2(0.f);
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

		void UpdatePartialLookupList(GrowingArray<CloudParticle>& cloudParticles, float hRadius);

		glm::vec2 CalculateForceToSdf(const CloudParticle& cloudParticle, const GrowingArray<float>& distanceField);
		
		void UpdateParticlesBasedOnPaper(GrowingArray<CloudParticle>& cloudParticleList, float h, glm::uvec2 resolution, float actualDT, const GrowingArray<float>& distanceField);
		
		void CalculateDensityForEachPointWithinRadius(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck, float h, float mass);
		glm::vec2 CalculateViscosityForceForEachPointWithinRadius(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck, float h, float mass);
		glm::vec2 CalculatePressureForceForEachPointWithinRadius(GrowingArray<CloudParticle>& cloudParticleList, uint32 indexToCheck, float h, float mass, float dT);
	private:
		float m_mouseForceStrength = 1.f;
		float m_mouseForceRadius = 0.05f;
		float m_particleSpacing = -0.1f;

		uint32 m_particleToDebug = 0u;

		float m_particleKernelRadius = 4.f;
		float m_mass = 1.0f;
		float m_restDensity = 5.0f;
		float m_nearPressureMultiplier = 20.f;
		float m_stiffness = 2.0f;
		float m_viscosity = 1.f;
		float m_gravity = 1.f;
		float m_maxVelocity = 1.f;
		float m_deltaTimeModifier = 0.4f;

		glm::ivec2 m_numberOfPointsToSpawn = glm::ivec2(5, 5);
		GrowingArray<CloudParticle> m_particleGrid;
		GrowingArray<CloudParticle> m_cloudParticles;

		uint32 m_particleGridSize = 128u;
		uint32 m_numberOfParticles;

		GrowingArray<SpatialIndexLookup> m_particleSpatialLookup;
		GrowingArray<uint32> m_particleSpatialLookupStartIndices;

		bool m_bSimulateGrid = true;
		bool m_bShowCircles = true;
		bool m_bShowGradientValues = true;
		bool m_bUpdatedParticleGrid = false;
	};

}

