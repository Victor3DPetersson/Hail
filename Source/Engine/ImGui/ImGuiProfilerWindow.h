#pragma once

namespace Hail
{
	class ImGuiFileBrowser;
	class ImGuiContext;
	class ResourceManager;

	class ImGuiProfilerWindow
	{
	public:

		void RenderImGuiCommands(ImGuiContext* context);

	private:

		uint32 m_numberOfFramesInCurrentSecond = 0;
		uint32 m_timeCounterInMs = 0u;

		float m_accumulatedDeltaTimeCurrentSecond = 0.f;
		float m_deltaTimeLastSecond = 0.f;
		float m_accumulatedDeltaTimeMsCurrentSecond = 0.f;
		float m_deltaTimeMsLastSecond = 0.f;

		// TODO, save averages and start plotting a graph of data over time
	};
}
