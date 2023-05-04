#pragma once
#include <thread>
#include <atomic>

#include "StartupAttributes.h"
#include "Resources\Resource.h"
#include "Resources\MaterialResources.h"
#include "Rendering\UniformBufferManager.h"
#include "Containers\VectonOnStack\VectorOnStack.h"
#include "Rendering\SwapChain.h"

#include "glm\vec2.hpp"
#include "String.hpp"



struct CompiledShader;
class Timer;
namespace Hail
{
	class RenderingDevice;
	class ResourceManager;
	struct RenderCommandPool;
	class FrameBufferTexture;
	struct RenderCommand_Sprite;
	struct RenderCommand_Mesh;
	class Renderer
	{
	public:

		virtual bool InitDevice(RESOLUTIONS startupResolution, Timer* timer) = 0;
		virtual bool InitGraphicsEngine(ResourceManager* resourceManager) = 0;
		virtual void StartFrame(RenderCommandPool& renderPool);
		virtual void EndFrame();
		virtual void Render();
		virtual void Cleanup() = 0;
		virtual void InitImGui() = 0; 

		//virtual FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format = TEXTURE_FORMAT::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED) = 0;
		//virtual void FrameBufferTexture_ClearFrameBuffer(FrameBufferTexture& frameBuffer) = 0;
		//virtual void FrameBufferTexture_BindAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) = 0;
		//virtual void FrameBufferTexture_SetAsRenderTargetAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) = 0;
		//virtual void FrameBufferTexture_EndRenderAsTarget(FrameBufferTexture& frameBuffer) = 0;

		virtual void BindMaterial(Material& materialToBind) = 0;
		virtual void EndMaterialPass() = 0;
		virtual void RenderSprite(const RenderCommand_Sprite& spriteCommandToRender, uint32_t spriteInstance) = 0;
		virtual void RenderMesh(const RenderCommand_Mesh& meshCommandToRender, uint32_t meshInstance) = 0;
		virtual void RenderLetterBoxPass() = 0;

		RenderingDevice* GetRenderingDevice() { return m_renderDevice; }

		void WindowSizeUpdated();
	protected:

		bool m_shadersRecompiled = false;
		bool m_framebufferResized = false;
		ResourceManager* m_resourceManager = nullptr;
		Timer* m_timer = nullptr;

		RenderingDevice* m_renderDevice = nullptr;
		RenderCommandPool* m_commandPoolToRender;
	};
}
