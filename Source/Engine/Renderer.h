#pragma once
#include <thread>
#include <atomic>

#include "StartupAttributes.h"
#include "ResourceCommon.h"
#include "Resources\MaterialResources.h"
#include "Rendering\SwapChain.h"

#include "glm\vec2.hpp"
#include "String.hpp"



struct CompiledShader;
namespace Hail
{
	struct RenderCommandPool;
	struct RenderData_Mesh;
	struct RenderCommand_Sprite;
	
	class BufferObject;
	class CloudRenderer;
	class DebugRenderingManager;
	class FontRenderer;
	class FrameBufferTexture;
	class RenderContext;
	class RenderingDevice;
	class ResourceManager;
	class Timer;
	
	class Renderer
	{
	public:
		~Renderer();
		// TODO: Clean up the initialization order of all the rendering systems, this is a mess atm.
		virtual bool Initialize();
		virtual void Cleanup();
		virtual bool InitDevice(Timer* timer) = 0;
		virtual bool InitGraphicsEngineAndContext(ResourceManager* resourceManager) = 0;
		//Always call virtual version of this function after swapchain has finished the previous frame
		virtual void StartFrame(RenderCommandPool& renderPool);
		void Prepare();
		void EndFrame();
		virtual void Render();
		virtual void InitImGui() = 0; 
		// Blocking operation for executing thread.
		virtual void WaitForGPU() = 0;

		//virtual FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, eTextureFormat format = eTextureFormat::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED) = 0;
		//virtual void FrameBufferTexture_ClearFrameBuffer(FrameBufferTexture& frameBuffer) = 0;
		//virtual void FrameBufferTexture_BindAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) = 0;
		//virtual void FrameBufferTexture_SetAsRenderTargetAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) = 0;
		//virtual void FrameBufferTexture_EndRenderAsTarget(FrameBufferTexture& frameBuffer) = 0;

		virtual void RenderMesh(const RenderData_Mesh& meshCommandToRender, uint32_t meshInstance) = 0;
		virtual void RenderImGui() = 0;

		RenderingDevice* GetRenderingDevice() { return m_renderDevice; }
		RenderContext* GetCurrentContext() { return m_pContext; }

	protected:

		void CreateSpriteVertexBuffer();
		void CreateVertexBuffer();
		void CreateIndexBuffer();

		BufferObject* m_pVertexBuffer = nullptr;
		BufferObject* m_pIndexBuffer = nullptr;
		BufferObject* m_pSpriteVertexBuffer = nullptr;

		bool m_shadersRecompiled = false;
		ResourceManager* m_pResourceManager = nullptr;
		Timer* m_timer = nullptr;

		// TODO: Should this be here? Figure out where we place our renderers and how to structure it properly.
		FontRenderer* m_pFontRenderer = nullptr;
		CloudRenderer* m_pCloudRenderer = nullptr;
		DebugRenderingManager* m_pDebugRenderingManager = nullptr;

		RenderingDevice* m_renderDevice = nullptr;
		RenderCommandPool* m_commandPoolToRender = nullptr;
		RenderContext* m_pContext = nullptr;
		uint64 m_currentlyBoundPipeline{};
	};
}
