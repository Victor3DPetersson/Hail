#include "Engine_PCH.h"
#include "ImGuiProfilerWindow.h"
#include "HailEngine.h"
#include "imgui.h"
#include "Timer.h"

void Hail::ImGuiProfilerWindow::RenderImGuiCommands(ImGuiContext* context)
{
	const Timer& renderLoopTimer = GetRenderLoopTimer();

	
	ImGui::BeginChild("Profiler", ImGui::GetContentRegionAvail(), true);

	ImGui::Text("Application total time : %fs", renderLoopTimer.GetTotalTime());
	ImGui::Text("Render loop delta time : %fs, %fms ", renderLoopTimer.GetDeltaTime(), renderLoopTimer.GetDeltaTimeMs());

	ImGui::EndChild();
}
