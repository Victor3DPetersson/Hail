#include "Engine_PCH.h"
#include "SwapChain.h"
#include "DebugMacros.h"

namespace Hail
{
	SwapChain::SwapChain(TextureManager* pTextureManager) : m_pTextureManager(pTextureManager)
	{
		// start with a 16/9 Aspect ratio, TODO: make configurable
		m_horizontalAspectRatio = 16.0f / 9.0f;
		m_bResizeSwapChain = false;
		m_windowResolution = { 0, 0 };
		m_renderTargetResolution = { 0, 0 };
		m_targetResolution = { 720, 480 };
		m_pFrameBufferTexture = nullptr;
	}

	void SwapChain::CalculateRenderResolution()
	{
		const float aspectRatio16x9 = (float)m_targetResolution.x / (float)m_targetResolution.y;

		m_horizontalAspectRatio = float(m_windowResolution.x) / float(m_windowResolution.y);
		if (m_horizontalAspectRatio >= aspectRatio16x9)
		{
			m_renderTargetResolution.x = m_windowResolution.y * aspectRatio16x9;
			m_renderTargetResolution.y = m_windowResolution.y;
		}
		else
		{
			m_renderTargetResolution.x = m_windowResolution.x;
			m_renderTargetResolution.y = m_windowResolution.x / aspectRatio16x9;
		}
		Debug_PrintConsoleStringL(StringL::Format("Window Res X: %i Window Res y: %i\nRender Target Res X: %i Render Target Res y: %i", m_windowResolution.x, m_windowResolution.y, m_renderTargetResolution.x, m_renderTargetResolution.y));
	}

	void SwapChain::SetTargetResolution(glm::uvec2 targetResolution)
	{
		if (targetResolution != m_targetResolution)
		{
			m_targetResolution = targetResolution;
			m_bResizeSwapChain = true;
		}
	}
	void SwapChain::SetWindowResolution(glm::uvec2 targetResolution)
	{
		if (targetResolution != m_windowResolution)
		{
			m_windowResolution = targetResolution;
			m_bResizeSwapChain = true;
		}
	}
}