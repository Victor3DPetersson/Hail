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
		glm::vec2 cloudTextureDimensions = { 40.f, 20.f };

		float graviticForce = -0.15f;
		float cloudDampeningMultiplier = 1.0f;
		float mouseForceStrength = 1.f;
		float mouseForceRadius = 0.025f;

		float mass = 1.0f;
		float stiffness = 2.0f;
		float restDensity = 1.5f;
		float nearPressureMultiplier = 2.f;

		float viscosityModifier = 1.f;
		float deltaTimeModifier = 0.4f;
		float particleKernelRadius = 4.f;
		uint32 bSimulateCloud  = 0u;

		glm::vec2 cloudTexturePosition = glm::vec2(25.0, 46.0);
		float effectTurbulence = 16.f;
		float effectStepLength = 10.f;

		float effectNoise = 0.05f;
		float effectSunDirectionRadian = 1.0f;
		float HenyeyGreensteinPhaseValue = .95f;
		float BeersLawStepLengthMultiplier = 8.0f;

		float tileableCloudThreshold = 0.05f;
		float tileableCloudTilingFactor = 10.f;
		int32 numberOfSteps = 4;
		float ditherThreshold = 0.2f;
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
		MaterialPipeline* m_pCollectJFAPipeline;

		TextureView* m_pTileableCloudTextureView;

		TextureResource* m_pSdfTexture;
		TextureView* m_pSdfView;

		// Double buffering
		StaticArray<TextureResource*, 2> m_pParticleCoverageTexture;
		StaticArray<TextureView*, 2> m_pParticleCoverageViewWrite;
		StaticArray<TextureView*, 2> m_pParticleCoverageViewRead;
		StaticArray<MaterialPipeline*, 2u> m_pJFAPipelines;


		GrowingArray<CloudParticle> m_cloudParticles;
		GrowingArray<CloudParticle> m_cloudGridParticles;
		GrowingArray<CloudParticle> m_cloudParticlesToSimulate;
		GrowingArray<float> m_cloudSdfTexture;

		uint32 m_numberOfPointsUploaded;

		CloudParticleSimulator m_simulator;

		ParticleUniformBuffer m_ParticleUniforms;

		// Temp variables, will remove later. 
		bool m_bUpdatedParticleGrid = false;
		bool m_bSimulateGrid = false;
		bool m_bRespawnGrid = false;
		glm::ivec2 m_numberOfPointsToSpawn = glm::ivec2(5, 5);
		float m_particleSpacing = 0.1f;
	};

}

