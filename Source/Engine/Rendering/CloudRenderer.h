#pragma once
#include "Types.h"
#include "ResourceCommon.h"


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

		void Prepare(const RenderCommandPool& poolOfCommands);
		void Render();

	private:
		Renderer* m_pRenderer;
		ResourceManager* m_pResourceManager;

		MaterialPipeline* m_pCloudPipeline;
		BufferObject* m_pCloudBuffer;
		TextureResource* m_pSdfTexture;
		TextureView* m_pSdfView;

		uint32 m_numberOfPointsUploaded;
	};

}

