#pragma once
#include "Types.h"
#include "ResourceCommon.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "CloudParticleSimulator.h"

namespace Hail
{
	class BufferObject;
	class MaterialPipeline;
	class Renderer;
	class ResourceManager;
	class TextureResource;
	class TextureView;

	struct RenderCommandPool;

	struct SortConstants
	{
		uint32 currentOffset;
		uint32 totalNumberOfElements;
		uint32 numberOfGroups;
		uint32 padding;
	};

	constexpr uint32 MaxNumberOfFluidParticles = 0xffff + 1u;
	// 16 is the number of Bits in our keyes, 4 is the number of bits per Radix Pass
	constexpr uint32 NumberOfDispatches = 16u / 4u;
	// 256 is the sorting workgroup size
	constexpr uint32 MaxNumberOfSortBuckets = MaxNumberOfFluidParticles / 256u;

	constexpr uint32 TestOffset = (512u >> 1u);
	constexpr uint32 Test = (3u) << TestOffset;

	struct ParticleUniformBuffer
	{
		uint32 numberOfParticles;
		float particleSize = 20.f;
		glm::vec2 cloudTextureDimensions = { 240.f, 240.f };

		float graviticForce = -1.0f;
		float cloudDampeningMultiplier = 0.5f;
		float mouseForceStrength = 1.f;
		float mouseForceRadius = 0.1f;

		float mass = 1.0f;
		float stiffness = 2.0f;
		float restDensity = 3.0f;
		float nearPressureMultiplier = 5.f;

		float viscosityModifier = 1.f;
		float deltaTimeModifier = 0.4f;
		float particleKernelRadius = 4.f;
		uint32 bSimulateCloud  = 0u;
	};

	struct ParticleDynamicVariableBuffer
	{
		float maxVelocity;
		uint32 numberOfParticles;
		glm::vec2 padding;
	};

	constexpr uint32 derp = MaxNumberOfFluidParticles << 1;

	class CloudRenderer
	{
	public:
		~CloudRenderer();
		explicit CloudRenderer(Renderer* pRenderer, ResourceManager* pResourceManager);
		bool Initialize();
		void Cleanup();

		void Prepare(RenderCommandPool& poolOfCommands);
		void Render();

	private:
		Renderer* m_pRenderer;
		ResourceManager* m_pResourceManager;

		// Buffers

		BufferObject* m_pParticleSortGlobalHistogramBuffer;
		BufferObject* m_pParticleSortLocalHistogramBuffer;
		BufferObject* m_pParticleSortBucketOffsetBuffer;

		BufferObject* m_pParticleBuffer;
		BufferObject* m_pParticleUploadBuffer;

		BufferObject* m_pParticleUniformBuffer;
		BufferObject* m_pDynamicParticleVariableBuffer;

		BufferObject* m_pParticleLookupBufferRead;
		BufferObject* m_pParticleLookupBufferWrite;
		BufferObject* m_pParticleLookupStartIndicesBuffer;
		BufferObject* m_pParticleLookupSortIndicesBufferRead;
		BufferObject* m_pParticleLookupSortIndicesBufferWrite;

		// Pipelines

		MaterialPipeline* m_pCloudPipeline;
		MaterialPipeline* m_pCloudCoveragePipeline;

		MaterialPipeline* m_pRadixBuildHistogramPipeline;
		MaterialPipeline* m_pRadixBuildOffsetTablePipeline;
		MaterialPipeline* m_pRadixShuffleDataPipeline;

		MaterialPipeline* m_pSimulationUpdateUniforms;
		MaterialPipeline* m_pSimulationUpdateParticleListFromCPU;
		MaterialPipeline* m_pSimulationCalculateDensity;
		MaterialPipeline* m_pSimulationCalculateIntermediateVelocity;
		MaterialPipeline* m_pSimulationCalculatePressureForce;
		MaterialPipeline* m_pSimulationApplyPosition;
		MaterialPipeline* m_pSimulationBuildPartialLookup;
		MaterialPipeline* m_pSimulationClearPartialLookup;

		TextureResource* m_pSdfTexture;
		TextureView* m_pSdfView;

		TextureResource* m_pParticleCoverageTexture;
		TextureView* m_pParticleCoverageViewWrite;
		TextureView* m_pParticleCoverageViewRead;

		GrowingArray<CloudParticle> m_cloudParticles;
		GrowingArray<CloudParticle> m_cloudGridParticles;
		GrowingArray<CloudParticle> m_cloudParticlesToSimulate;
		GrowingArray<float> m_cloudSdfTexture;

		uint32 m_numberOfPointsUploaded;

		CloudParticleSimulator m_simulator;

		ParticleUniformBuffer m_ParticleUniforms;

		// Temp variables, will remove later. 
		bool m_bUpdatedParticleGrid = false;
		bool m_bSimulateGrid = true;
		bool m_bRespawnGrid = false;
		glm::ivec2 m_numberOfPointsToSpawn = glm::ivec2(5, 5);
		float m_particleSpacing = 0.1f;
	};

}

