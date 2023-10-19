#pragma once
#include "glm\vec2.hpp"
#include "Types.h"

namespace Hail
{
	class RenderingDevice;
	class FrameBufferTexture;

	class SwapChain
	{
	public:
		SwapChain() = default;
		virtual void Init(RenderingDevice* renderDevice) = 0;
		virtual void DestroySwapChain(RenderingDevice* renderDevice) = 0;
		virtual FrameBufferTexture* GetFrameBufferTexture() = 0;
		//On systems without any frames in flight this will always return 0
		virtual uint32 GetFrameInFlight() = 0;
		glm::uvec2 GetSwapChainResolution() { return m_windowResolution; }
		glm::uvec2 GetRenderTargetResolution() { return m_renderTargetResolution; }

		void SetTargetResolution(glm::uvec2 targetResolution);
	protected:
		void CalculateRenderResolution();

		glm::uvec2 m_windowResolution = {0, 0};
		glm::uvec2 m_renderTargetResolution = {0, 0};
		glm::uvec2 m_targetResolution = { 720, 480 };

		bool m_resizeSwapChain = false;
	};
}

