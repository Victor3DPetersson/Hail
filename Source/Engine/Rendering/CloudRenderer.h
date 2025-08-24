#pragma once
#include "Types.h"
#include "ResourceCommon.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "CloudParticleSimulator.h"

namespace Hail
{
	class BufferObject;
	class ErrorManager;
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

	constexpr uint32 MaxNumberOfClouds = 16u;

	struct ParticleUniformBuffer
	{
		uint32 numberOfParticles;
		float cloudSizeMultiplier = 16.f;
		float graviticForce = -0.25f;
		float cloudDampeningMultiplier = 1.0f;

		float mouseForceStrength = 1.f;
		float mouseForceRadius = 0.025f;
		float mass = 1.0f;
		float stiffness = 2.0f;

		float restDensity = -0.195f;
		float nearPressureMultiplier = 0.5f;
		float viscosityModifier = 20.f;
		float deltaTimeModifier = 0.4f;

		float particleKernelRadius = 6.f;
		uint32 bSimulateCloud  = 1u;
		float effectTurbulence = 32.f;
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

	struct CloudData
	{
		glm::vec2 position = glm::vec2(0.f);
		glm::vec2 dimensions = { 64.f, 32.f };
	};  


	struct ParticleDynamicVariableBuffer
	{
		float maxVelocity = 0.f;
		uint32 numberOfParticles;
		glm::vec2 padding;
	};

	class CloudRenderer
	{
	public:
		~CloudRenderer();
		explicit CloudRenderer(Renderer* pRenderer, ResourceManager* pResourceManager);
		void Initialize(ErrorManager* pErrorManager);
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

		BufferObject* m_pParticleUniformBuffer;
		BufferObject* m_pDynamicParticleVariableBuffer;

		BufferObject* m_pParticleLookupBufferRead;
		BufferObject* m_pParticleLookupBufferWrite;
		BufferObject* m_pParticleLookupStartIndicesBuffer;
		BufferObject* m_pParticleLookupSortIndicesBufferRead;
		BufferObject* m_pParticleLookupSortIndicesBufferWrite;

		BufferObject* m_pCloudListBuffer;
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
		GrowingArray<CloudData> m_cloudList;
		GrowingArray<float> m_cloudSdfTexture;

		uint32 m_numberOfPointsUploaded;

		CloudParticleSimulator m_simulator;

		ParticleUniformBuffer m_ParticleUniforms;

		// Temp variables, will remove later. 
		int32 m_numberOfClouds = 10;
		int32 m_numberOfPointsPerPixelInClouds = 2;
		bool m_bRespawnParticles = false;

		bool m_bUpdatedParticleGrid = false;
		bool m_bSimulateGrid = false;
	};

}

