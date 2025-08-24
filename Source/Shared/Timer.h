#pragma once
#include "Types.h"

namespace Hail
{
    class Timer
    {
    public:
        Timer();
        Timer(const Timer& timer) = delete;
        Timer& operator=(const Timer& timer) = delete;
        Timer(const Timer&& timer) = delete;
        Timer& operator=(const Timer&& timer) = delete;

        void FrameStart();
        double GetDeltaTime() const;
        float GetDeltaTimeMs() const { return m_deltaTimeMs; }
        double GetTotalTime() const;
        uint64 GetTotalTimeMs() const;
        // System time in nano seconds
        uint64 GetSystemTime() const;
    private:
        float m_deltaTimeMs = 0;
        uint32 m_deltaTimeMsUint = 0;
        uint64 m_startTimeMicroSec = 0;
        uint64 m_frameStartMicroSec = 0;
    };
}
