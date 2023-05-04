#include "Engine_PCH.h"
#include "SwapChain.h"
#include "DebugMacros.h"

namespace Hail
{
	void SwapChain::CalculateRenderResolution()
	{
		const float aspectRatio16x9 = 16.0f / 9.0f;

		const float ratioX = float(m_windowResolution.x) / float(m_windowResolution.y);
		if (ratioX > aspectRatio16x9)
		{
			m_renderTargetResolution.x = m_windowResolution.y * aspectRatio16x9;
			m_renderTargetResolution.y = m_windowResolution.y;
		}
		else
		{
			m_renderTargetResolution.x = m_windowResolution.x;
			m_renderTargetResolution.y = m_windowResolution.x / aspectRatio16x9;
		}
		Debug_PrintConsoleString256(String256::Format("Window Res X: %i Window Res y: %i\nRender Target Res X: %i Render Target Res y: %i", m_windowResolution.x, m_windowResolution.y, m_renderTargetResolution.x, m_renderTargetResolution.y));
	}
	void SwapChain::SetTargetResolution(glm::uvec2 targetResolution)
	{
		if (targetResolution != m_targetResolution)
		{
			m_targetResolution = targetResolution;
			m_resizeSwapChain = true;
		}
	}
}