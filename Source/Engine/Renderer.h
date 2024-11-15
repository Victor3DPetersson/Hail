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
	struct RenderCommand_Mesh;
	struct RenderCommand_Sprite;
	
	class FontRenderer;
	class FrameBufferTexture;
	class RenderContext;
	class RenderingDevice;
	class ResourceManager;
	class Timer;
	
	class Renderer
	{
	public:

		bool Initialize();

		virtual bool InitDevice(Timer* timer) = 0;
		virtual bool InitGraphicsEngine(ResourceManager* resourceManager) = 0;
		//Always call virtual version of this function after swapchain has finished the previous frame
		virtual void StartFrame(RenderCommandPool& renderPool);
		virtual void EndFrame();
		virtual void Render();
		virtual void Cleanup() = 0;
		virtual void InitImGui() = 0; 
		// Blocking operation for executing thread.
		virtual void WaitForGPU() = 0;

		//virtual FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format = TEXTURE_FORMAT::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED) = 0;
		//virtual void FrameBufferTexture_ClearFrameBuffer(FrameBufferTexture& frameBuffer) = 0;
		//virtual void FrameBufferTexture_BindAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) = 0;
		//virtual void FrameBufferTexture_SetAsRenderTargetAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) = 0;
		//virtual void FrameBufferTexture_EndRenderAsTarget(FrameBufferTexture& frameBuffer) = 0;

		virtual void BindMaterialPipeline(Pipeline* pipelineToBind, bool bFirstMaterialInFrame) = 0;
		virtual void EndMaterialPass() = 0;
		virtual void RenderSprite(const RenderCommand_Sprite& spriteCommandToRender, uint32_t spriteInstance) = 0;
		virtual void RenderMesh(const RenderCommand_Mesh& meshCommandToRender, uint32_t meshInstance) = 0;
		virtual void RenderDebugLines2D(uint32 numberOfLinesToRender, uint32 offsetFrom3DLines) = 0;
		virtual void RenderDebugLines3D(uint32 numberOfLinesToRender) = 0;
		virtual void RenderLetterBoxPass() = 0;
		virtual void RenderMeshlets(glm::uvec3 dispatchSize) = 0;

		RenderingDevice* GetRenderingDevice() { return m_renderDevice; }
		RenderContext* GetCurrentContext() { return m_pContext; }

		void WindowSizeUpdated();
	protected:

		bool m_shadersRecompiled = false;
		bool m_framebufferResized = false;
		ResourceManager* m_resourceManager = nullptr;
		Timer* m_timer = nullptr;

		// TODO: Should this be here? Figure out where we place our renderers and how to structure it properly.
		FontRenderer* m_pFontRenderer = nullptr;

		RenderingDevice* m_renderDevice = nullptr;
		RenderCommandPool* m_commandPoolToRender;
		RenderContext* m_pContext = nullptr;
		uint64 m_currentlyBoundPipeline{};
	};
}
