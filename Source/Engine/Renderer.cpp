#include "Engine_PCH.h"
#include "Renderer.h"
#include "DebugMacros.h"

void Hail::Renderer::ReloadShaders()
{
	m_shadersRecompiled = true;
}

void Hail::Renderer::CalculateRenderResolution(glm::uvec2 windowResolution)
{
	m_windowResolution = windowResolution;
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

void Hail::Renderer::WindowSizeUpdated()
{
	m_framebufferResized = true;
}
