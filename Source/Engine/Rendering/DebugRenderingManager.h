#pragma once

#include "Types.h"
#include "Resources\BufferResource.h"

#include "Containers\VectorOnStack\VectorOnStack.h"

namespace Hail
{
	class BufferObject;
	class MaterialPipeline;
	class Renderer;
	class ResourceManager;
	class TextureResource;
	class TextureView;

	struct RenderCommandPool;



	class DebugRenderingManager
	{
	public:
		explicit DebugRenderingManager(Renderer* pRenderer, ResourceManager* pResourceManager);
		~DebugRenderingManager();
		bool Initialize();
		void Cleanup();

		void Prepare(RenderCommandPool& poolOfCommands);
		void Render();

	private:
		Renderer* m_pRenderer;
		ResourceManager* m_pResourceManager;

		MaterialPipeline* m_pDebugCirclePipeline;
		BufferObject* m_pDebugCircleBuffer;
		uint32 m_numberOfCirclesToRender;
		// TODO: move debug line rendering to this class

		BufferObject* m_pDebugLineVertexBuffer = nullptr;
		BufferObject* m_pDebugLineBuffer = nullptr;
		MaterialPipeline* m_pDebugLinePipeline = nullptr;

		uint32 m_numberOf2DDebugLines;
		uint32 m_numberOf3DDebugLines;

		struct DebugLineData
		{
			glm::vec4 posAndIs2D;
			glm::vec4 color;
		};

		VectorOnStack<DebugLineData, MAX_NUMBER_OF_DEBUG_LINES, false> m_debugLineData;
	};

}