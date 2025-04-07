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

		// TODO, save averages and start plotting a graph of data over time
	};
}
