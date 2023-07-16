#include "Engine_PCH.h"
#include "ImGuiMaterialEditor.h"
#include "ImGuiFileBrowser.h"

#include "imgui.h"

#include "Resources\MaterialResources.h"

namespace Hail
{
	Material g_material;
	bool g_fileBrowserIsInited = false;
	bool g_openMaterialBrowser = false;
	bool g_inited = false;
	ImGuiFileBrowserData g_materialFileBrowserData;
	uint32_t g_selectedMaterialType = 0;

	StaticArray<String64, static_cast<uint32_t>(MATERIAL_TYPE::COUNT)> g_materialTypeStrings;

	void InitCommonData()
	{
		if (g_inited)
		{
			return;
		}
		g_inited = true;

		g_materialTypeStrings[static_cast<uint32_t>(MATERIAL_TYPE::SPRITE)] = "Sprite";
		g_materialTypeStrings[static_cast<uint32_t>(MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX)] = "Fullscreen Effect";
		g_materialTypeStrings[static_cast<uint32_t>(MATERIAL_TYPE::MODEL3D)] = "Model";
	}

	void InitFileBrowser()
	{
		if (g_fileBrowserIsInited)
		{
			return;
		}
		g_fileBrowserIsInited = true;
		g_materialFileBrowserData.allowMultipleSelection = false;
		g_materialFileBrowserData.objectsToSelect.Init(16);
		g_materialFileBrowserData.extensionsToSearchFor = { "mat" };
		g_materialFileBrowserData.pathToBeginSearchingIn = RESOURCE_DIR;
	}



}


void Hail::ImGuiMaterialEditor::RenderImGuiCommands(ImGuiFileBrowser* fileBrowser)
{
	InitCommonData();
	if (g_openMaterialBrowser)
	{

		bool finishedBrowsing = false;
		fileBrowser->RenderImGuiCommands(finishedBrowsing);
		if (finishedBrowsing)
		{
			g_openMaterialBrowser = false;
		}
	}
	if (ImGui::Button("Create"))
	{
		g_material.m_type = MATERIAL_TYPE::SPRITE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Load"))
	{
		InitFileBrowser();
		if (fileBrowser->Init(&g_materialFileBrowserData))
		{
			g_openMaterialBrowser = true;
		}
	}
	if (g_material.m_type != MATERIAL_TYPE::COUNT)
	{

		if (ImGui::BeginCombo("Render Style", g_materialTypeStrings[g_selectedMaterialType]))
		{
			for (int n = 0; n < static_cast<uint32_t>(MATERIAL_TYPE::COUNT); n++)
			{
				const bool is_selected = (g_selectedMaterialType == n);
				if (ImGui::Selectable(g_materialTypeStrings[n], is_selected))
				{
					g_selectedMaterialType = n;
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		//if()

	}
}

