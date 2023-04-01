#pragma once
#include <thread>
#include <atomic>

#include "StartupAttributes.h"
#include "Resources\Resource.h"
#include "Rendering\UniformBufferManager.h"
#include "Containers\VectonOnStack\VectorOnStack.h"

#include "glm\vec2.hpp"
#include "String.hpp"



struct CompiledShader;
class ShaderManager;
class TextureManager;
class Timer;
namespace Hail
{
	class ResourceManager;
	struct RenderCommandPool;
	class FrameBufferTexture;
	class Renderer
	{
	public:

		virtual bool Init(RESOLUTIONS startupResolution, ShaderManager* shaderManager, TextureManager* textureManager, ResourceManager* resourceManager, Timer* timer) = 0;
		virtual void StartFrame(RenderCommandPool& renderPool);
		virtual void EndFrame();
		virtual void Render() = 0;
		virtual void Cleanup() = 0;
		virtual void InitImGui() = 0;
		virtual void CreateShaderObject(CompiledShader& shader) = 0;

		virtual FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format = TEXTURE_FORMAT::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED) = 0;
		virtual void FrameBufferTexture_ClearFrameBuffer(FrameBufferTexture& frameBuffer) = 0;
		virtual void FrameBufferTexture_BindAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) = 0;
		virtual void FrameBufferTexture_SetAsRenderTargetAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) = 0;
		virtual void FrameBufferTexture_EndRenderAsTarget(FrameBufferTexture& frameBuffer) = 0;

		virtual void ReloadShaders();

		void WindowSizeUpdated();
		void SetTargetResolution(glm::uvec2 targetResolution) { m_targetResolution = targetResolution; }
	protected:
		void CalculateRenderResolution(glm::uvec2 windowResolution);


		bool m_shadersRecompiled = false;
		bool m_framebufferResized = false;
		ShaderManager* m_shaderManager = nullptr;
		TextureManager* m_textureManager = nullptr;
		ResourceManager* m_resourceManqager = nullptr;
		Timer* m_timer = nullptr;
		glm::uvec2 m_windowResolution;
		glm::uvec2 m_renderTargetResolution;
		glm::uvec2 m_targetResolution = { 720, 480 };

		FrameBufferTexture* m_mainPassFrameBufferTexture = nullptr;
		VectorOnStack<SpriteInstanceData, MAX_NUMBER_OF_SPRITES, false> m_spriteInstanceData;


	};
}
