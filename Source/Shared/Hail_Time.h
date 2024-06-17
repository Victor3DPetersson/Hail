#pragma once
#include "Timer.h"

namespace Hail
{
	void SetGlobalTimer(Timer* pTimer);
	Timer* GetGlobalTimer();
}