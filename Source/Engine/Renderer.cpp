#include "Engine_PCH.h"
#include "Renderer.h"

void Hail::Renderer::ReloadShaders()
{
	m_shadersRecompiled = true;
}

void Hail::Renderer::WindowSizeUpdated()
{
	m_framebufferResized = true;
}
