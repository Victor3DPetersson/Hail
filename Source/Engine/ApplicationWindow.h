#pragma once
#include "StartupAttributes.h"
struct ApplicationMessage
{
	uint32_t command;
	RESOLUTIONS renderResolution = RESOLUTIONS::COUNT;
	RESOLUTIONS windowResolution = RESOLUTIONS::COUNT;
};

class SlaskEngine;
class InputHandler;

class ApplicationWindow
{
public:

	virtual bool Init(StartupAttributes startupData, InputHandler* inputHandler) = 0;

	virtual void SetApplicationSettings(ApplicationMessage message) = 0;

	virtual void ApplicationUpdateLoop() = 0;

	virtual Vector2ui GetWindowResolution() = 0;
	virtual Vector2ui GetWindowPosition() = 0;
	virtual Vector2ui GetMonitorResolution() = 0;

	void ShutDown() { m_runApplication = false; }

	bool GetIsFullscreen() { return m_isFullScreen; }

protected:
	//v2ui GetWindowBorderSize();

	bool m_isFullScreen = false;
	bool m_hasBorder = true;
	bool m_runApplication = true;

	Vector2ui m_previousSize;
};