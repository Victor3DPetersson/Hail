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
        uint32 GetDeltaTimeMs() const { return m_deltaTimeMs; }
        double GetTotalTime() const;
        uint64 GetTotalTimeMs() const;
        uint64 GetSystemTime() const;
    private:
        uint32 m_deltaTimeMs = 0;
        uint64 m_startTimeMs = 0;
        uint64 m_frameStartMs = 0;
    };
}
