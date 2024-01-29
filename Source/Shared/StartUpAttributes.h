#pragma once
#include <functional>
#include <string>
#include "Utilities.h"

constexpr uint32_t INVALID_UINT = 0xffffffff;

namespace Hail
{
	struct ApplicationFrameData;
}

using callback_function = std::function<void()>;
using callback_function_init = std::function<void(void*)>;
using callback_function_totalTime_dt_frmData = std::function<void(double, float, Hail::ApplicationFrameData&)>;


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
	callback_function_init initFunctionToCall = nullptr;
	callback_function postInitFunctionToCall = nullptr;
	callback_function_totalTime_dt_frmData updateFunctionToCall = nullptr;
	callback_function shutdownFunctionToCall = nullptr;

	RESOLUTIONS startupResolution = RESOLUTIONS::RES720;

	uint16_t startPositionX = 400;
	uint16_t startPositionY = 400;

	uint8_t applicationTickRate = 60;

	//std::wstring applicationName;

	bool startInFullScreen = false;
};