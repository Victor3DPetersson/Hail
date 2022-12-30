#pragma once
#include <functional>
#include <string>
#include "Utilities.h"

using callback_function = std::function<void()>;
using callback_function_dt = std::function<void(float)>;



enum APPLICATION_COMMAND
{
	NONE = 1 << 0,
	FULLSCREEN = 1 << 1, //Will toggle fullscreen
	CHANGE_RESOLUTION = 1 << 2,
	CHANGE_WINDOW_RESOLUTION = 1 << 3
};

struct ApplicationMessage
{
	uint32_t command;
	RESOLUTIONS renderResolution = RESOLUTIONS::COUNT;
	RESOLUTIONS windowResolution = RESOLUTIONS::COUNT;
};

struct StartupAttributes
{
	callback_function initFunctionToCall = nullptr;
	callback_function_dt updateFunctionToCall = nullptr;
	callback_function_dt renderFunctionToCall = nullptr;
	callback_function shutdownFunctionToCall = nullptr;

	RESOLUTIONS startupResolution = RESOLUTIONS::RES720;

	uint16_t startPositionX = 400;
	uint16_t startPositionY = 400;

	//std::wstring applicationName;

	bool startInFullScreen = false;
};