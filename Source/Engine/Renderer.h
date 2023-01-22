//Interface for the entire engine
#pragma once
#include <thread>
#include <atomic>

#include "StartupAttributes.h"

#include "glm\vec2.hpp"

struct CompiledShader;
class ShaderManager;
class TextureManager;
class Timer;
namespace Hail
{
	class Renderer
	{
	public:

		virtual bool Init(RESOLUTIONS startupResolution, ShaderManager* shaderManager, TextureManager* textureManager, Timer* timer) = 0;
		virtual void StartFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Render() = 0;
		virtual void Cleanup() = 0;
		virtual void InitImGui() = 0;
		virtual void RecreateSwapchain() = 0;
		virtual void CreateShaderObject(CompiledShader& shader) = 0;

		void WindowSizeUpdated();
	protected:



		bool m_framebufferResized = false;
		glm::uvec2 m_renderResolution;
		ShaderManager* m_shaderManager = nullptr;
		TextureManager* m_textureManager = nullptr;
		Timer* m_timer = nullptr;
	};
}
