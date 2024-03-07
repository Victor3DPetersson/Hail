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
		SwapChain();
		virtual void Init(RenderingDevice* renderDevice) = 0;
		virtual void DestroySwapChain(RenderingDevice* renderDevice) = 0;
		virtual FrameBufferTexture* GetFrameBufferTexture() = 0;
		//On systems without any frames in flight this will always return 0
		virtual uint32 GetFrameInFlight() = 0;
		glm::uvec2 GetSwapChainResolution() const { return m_windowResolution; }
		glm::uvec2 GetRenderTargetResolution() const { return m_renderTargetResolution; }
		float GetHorizontalAspectRatio() const { return m_horizontalAspectRatio; }
		void SetTargetResolution(glm::uvec2 targetResolution);
		void SetWindowResolution(glm::uvec2 targetResolution);

	protected:
		void CalculateRenderResolution();

		glm::uvec2 m_windowResolution;
		glm::uvec2 m_renderTargetResolution;
		glm::uvec2 m_targetResolution;

		float m_horizontalAspectRatio;
		bool m_bResizeSwapChain;
	};
}

