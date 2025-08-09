#pragma once

namespace Hail
{
	class ImGuiFileBrowser;
	class ImGuiContext;
	class ResourceManager;

	enum class ImGuiPropertyWindowReturnValue
	{
		NoOp,
		OpenMaterialWindow,
		ReloadResource
	};


	class ImGuiPropertyWindow
	{
	public:

		ImGuiPropertyWindowReturnValue RenderImGuiCommands(ImGuiFileBrowser* fileBrowser, ImGuiContext* context);


	private:


	};
}


