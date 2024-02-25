#pragma once
#include "StartupAttributes.h"

namespace Hail
{
	class InputHandler;
}

class ApplicationWindow
{
public:

	virtual bool Init(StartupAttributes startupData, Hail::InputHandler* inputHandler) = 0;

	virtual void SetApplicationSettings(Hail::ApplicationMessage message) = 0;

	virtual void ApplicationUpdateLoop() = 0;

	virtual glm::uvec2 GetWindowResolution() = 0;
	virtual glm::uvec2 GetWindowPosition() = 0;
	virtual glm::uvec2 GetMonitorResolution() = 0;

	bool GetIsFullscreen() { return m_isFullScreen; }

protected:
	//v2ui GetWindowBorderSize();

	bool m_isFullScreen = false;
	bool m_hasBorder = true;
	bool m_previousHasBorder = m_hasBorder;
	bool m_hasFocus = true;

	glm::uvec2 m_previousSize;
	glm::uvec2 m_windowSize;
	glm::uvec2 m_frameBufferSize;
	glm::uvec2 m_previousFrameBufferSize;
};