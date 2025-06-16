#pragma once
#include "glm\vec2.hpp"
#include "Types.h"
#include "ResourceCommon.h"
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{
	class RenderingDevice;
	class FrameBufferTexture;
	class TextureView;
	class TextureManager;

	class SwapChain
	{
	public:
		explicit SwapChain(TextureManager* pTextureManager);
		virtual void Init(RenderingDevice* renderDevice) = 0;
		virtual void DestroySwapChain(RenderingDevice* renderDevice);
		FrameBufferTexture* GetFrameBufferTexture() { return m_pFrameBufferTexture; }
		virtual TextureView* GetSwapchainView() = 0;
		//On systems without any frames in flight this will always return 0
		virtual uint32 GetFrameInFlight() = 0;
		glm::uvec2 GetSwapChainResolution() const { return m_windowResolution; }
		// The resolution of the rendertarget after blackboarding and window resize
		glm::uvec2 GetRenderTargetResolution() const { return m_renderTargetResolution; }
		// The resolution of the main render target
		glm::uvec2 GetTargetResolution() const { return m_targetResolution; }
		float GetTargetHorizontalAspectRatio() const { return m_targetHorizontalAspectRatio; }
		float GetHorizontalAspectRatio() const { return m_horizontalAspectRatio; }
		void SetTargetResolution(glm::uvec2 targetResolution);
		void SetWindowResolution(glm::uvec2 targetResolution);

	protected:
		void CalculateRenderResolution();

		TextureManager* m_pTextureManager;
		glm::uvec2 m_windowResolution;
		glm::uvec2 m_renderTargetResolution;
		glm::uvec2 m_targetResolution;

		float m_horizontalAspectRatio;
		float m_targetHorizontalAspectRatio;
		bool m_bResizeSwapChain;
		FrameBufferTexture* m_pFrameBufferTexture;
		GrowingArray<TextureView*> m_pTextureViews;
	};
}

