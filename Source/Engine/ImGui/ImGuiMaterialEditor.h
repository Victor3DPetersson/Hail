#pragma once
#include "ImGuiContext.h"
namespace Hail
{
	class ImGuiFileBrowser;
	class ResourceManager;

	class ImGuiMaterialEditor
	{
	public:
		ImGuiMaterialEditor();
		void RenderImGuiCommands(ImGuiFileBrowser* fileBrowser, ResourceManager& resourceManager, ImGuiContext* contextObject, bool* closeButton);
		void SetMaterialAsset(ImGuiContext* context);
	private:
		MaterialResourceContextObject m_materialToEdit;
		bool m_bOpenFileBrowser;
		uint8 m_selectedShader;
		uint8 m_selectedTextureSlot;
	};
}
