#include "Shared_PCH.h"
#include "Timer.h"
#include <chrono>

#ifdef PLATFORM_WINDOWS
#include "windows.h"
#endif

using namespace Hail;

uint64 localGetCurrentTimeInMicroSec()
{
	const auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
	return (uint64)std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

Hail::Timer::Timer()
{
	m_startTimeMicroSec = localGetCurrentTimeInMicroSec();
	m_frameStartMicroSec = m_startTimeMicroSec;
}

void Timer::FrameStart()
{
	const uint64 currentTimeInMicroSec = localGetCurrentTimeInMicroSec();

	m_deltaTimeMs = (currentTimeInMicroSec - m_frameStartMicroSec) * 0.001f;
	m_deltaTimeMsUint = (uint32)m_deltaTimeMs;
	m_frameStartMicroSec = currentTimeInMicroSec;
}

double Timer::GetDeltaTime() const
{
	return m_deltaTimeMs * 0.001;
}

double Timer::GetTotalTime() const
{
	return (localGetCurrentTimeInMicroSec() - m_startTimeMicroSec) * 0.000001;
}

uint64 Timer::GetTotalTimeMs() const
{
	return (localGetCurrentTimeInMicroSec() - m_startTimeMicroSec) / 1000;
}

uint64 Hail::Timer::GetSystemTime() const
{
	std::chrono::time_point<std::chrono::system_clock> timeZoneBias;
#ifdef PLATFORM_WINDOWS
	TIME_ZONE_INFORMATION timeZoneData;
	bool hasBias = GetTimeZoneInformation(&timeZoneData);
	if (hasBias)
	{
		int totalBias = timeZoneData.Bias + timeZoneData.DaylightBias;
		timeZoneBias -= std::chrono::minutes(totalBias);
	}
#endif
	
	const std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + timeZoneBias.time_since_epoch();
	// Magic number that might only be needed on windows? Will be vigilant with time for now.
	return timePoint.time_since_epoch().count() + 116444736000000000;
}
