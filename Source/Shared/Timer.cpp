#include "Shared_PCH.h"
#include "Timer.h"

void Timer::FrameStart()
{
	std::chrono::duration<double, std::ratio<1, 1>> deltaTime = std::chrono::high_resolution_clock::now() - m_frameStart;
	if (m_isPaused)
	{
		m_pausedTotalTime += (deltaTime.count());
	}
	else
	{
		m_deltaTime = (deltaTime.count());
	}
	m_frameStart = std::chrono::high_resolution_clock::now();
}
double Timer::GetDeltaTime() const
{
	return m_deltaTime;
}
double Timer::GetPausedTotalTime() const
{
	return m_pausedTotalTime;
}
double Timer::GetTotalTime() const
{
	std::chrono::duration<double, std::ratio<1, 1>> totalTime = m_frameStart - m_begin;
	return totalTime.count();
}


void Timer::Pause()
{
	m_isPaused = true;
}

void Timer::Resume()
{
	m_isPaused = false;
}
