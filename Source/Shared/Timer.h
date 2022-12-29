#pragma once
#include <chrono>

class Timer
{
public:
    Timer()
    {
        m_frameStart = std::chrono::high_resolution_clock::now();
        m_begin = std::chrono::high_resolution_clock::now();
    }
    Timer(const Timer& timer) = delete;
    Timer& operator=(const Timer& timer) = delete;
    Timer(const Timer&& timer) = delete;
    Timer& operator=(const Timer&& timer) = delete;

    void FrameStart();
    double GetDeltaTime() const;
    double GetPausedTotalTime() const;
    double GetTotalTime() const;
    void Pause();
    void Resume();
private:
    double m_deltaTime = 0;
    double m_pausedTotalTime = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_frameStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_begin;
    bool m_isPaused = false;
};