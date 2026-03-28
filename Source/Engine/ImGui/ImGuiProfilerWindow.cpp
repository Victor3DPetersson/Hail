#include "Engine_PCH.h"
#include "ImGuiProfilerWindow.h"
#include "HailEngine.h"
#include "imgui.h"
#include "Timer.h"

void Hail::ImGuiProfilerWindow::RenderImGuiCommands(ImGuiContext* context)
{
	const Timer& renderLoopTimer = GetRenderLoopTimer();

	m_numberOfFramesInCurrentSecond++;
	m_timeCounterInMs += renderLoopTimer.GetDeltaTimeMsUint();

	const float currentDeltaTime = renderLoopTimer.GetDeltaTime();
	const float currentDeltaTimeMs = renderLoopTimer.GetDeltaTimeMs();

	m_accumulatedDeltaTimeCurrentSecond += currentDeltaTime;
	m_accumulatedDeltaTimeMsCurrentSecond += currentDeltaTimeMs;

	if (m_timeCounterInMs >= 1000u)
	{
		m_timeCounterInMs = 0u;

		m_deltaTimeLastSecond = m_accumulatedDeltaTimeCurrentSecond / (float)m_numberOfFramesInCurrentSecond;
		m_deltaTimeMsLastSecond = m_accumulatedDeltaTimeMsCurrentSecond / (float)m_numberOfFramesInCurrentSecond;

		m_accumulatedDeltaTimeCurrentSecond = 0u;
		m_accumulatedDeltaTimeMsCurrentSecond = 0u;
		m_numberOfFramesInCurrentSecond = 0u;
	}

	
	ImGui::BeginChild("Profiler", ImGui::GetContentRegionAvail(), true);

	ImGui::Text("Application total time : %fs", renderLoopTimer.GetTotalTime());
	ImGui::Text("Render loop delta time : %fs, %fms ", currentDeltaTime, currentDeltaTimeMs);
	ImGui::Text("Render loop average delta time /s : %fs, %fms ", m_deltaTimeLastSecond, m_deltaTimeMsLastSecond);

	ImGui::EndChild();
}
