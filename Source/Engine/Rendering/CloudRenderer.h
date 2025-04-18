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

		MaterialPipeline* m_pCloudPipeline;
		BufferObject* m_pCloudBuffer;
		TextureResource* m_pSdfTexture;
		TextureView* m_pSdfView;

		GrowingArray<glm::vec2> m_pointsOnTheGPU;
		GrowingArray<CloudParticle> m_cloudParticles;
		GrowingArray<float> m_cloudSdfTexture;

		uint32 m_numberOfPointsUploaded;

		CloudParticleSimulator m_simulator;
	};

}

