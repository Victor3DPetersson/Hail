#pragma once
#include <functional>
#include <string>
#include "Utilities.h"

constexpr uint32_t INVALID_UINT = 0xffffffff;

using callback_function = std::function<void()>;
using callback_function_dt = std::function<void(float)>;


namespace Hail
{
	enum APPLICATION_COMMAND : uint32_t
	{
		NONE = 1 << 0,
		TOGGLE_FULLSCREEN = 1 << 1, //Will toggle fullscreen
		SET_RESOLUTION = 1 << 2,
		TOGGLE_BORDER = 1 << 3,
		MINIMIZE = 1 << 4,
		MAXIMIZE = 1 << 5,
		RESTORE = 1 << 6,
		MOVE_WINDOW = 1 << 7,
		DROP_FOCUS = 1 << 8,
		RESTORE_FOCUS = 1 << 9
	};

	struct ApplicationMessage
	{
		uint32_t command = 0;
		RESOLUTIONS renderResolution = RESOLUTIONS::COUNT;
		RESOLUTIONS windowResolution = RESOLUTIONS::COUNT;
	};
}


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