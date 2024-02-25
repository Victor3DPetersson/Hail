#include "Engine_PCH.h"
#include "ImGuiMaterialEditor.h"
#include "ImGuiFileBrowser.h"

#include "imgui.h"

#include "Resources\MaterialResources.h"
#include "ImGuiHelpers.h"

#include "HailEngine.h"
#include "Resources\ResourceRegistry.h"
#include "Resources\ResourceManager.h"
#include "Resources\MaterialManager.h"
#include "Resources\TextureManager.h"

namespace Hail
{
	bool g_inited = false;
	ImGuiFileBrowserData g_textureFileBrowserData;

	void InitCommonData()
	{
		if (g_inited)
		{
			return;
		}
		g_inited = true;

		g_textureFileBrowserData.allowMultipleSelection = false;
		g_textureFileBrowserData.objectsToSelect.Prepare(4);
		g_textureFileBrowserData.extensionsToSearchFor = { "txr" };
		g_textureFileBrowserData.pathToBeginSearchingIn = FilePath::GetCurrentWorkingDirectory();
	}
}


Hail::ImGuiMaterialEditor::ImGuiMaterialEditor()
{
	m_openTextureBrowser = false;
	m_selectedTextureSlot = 0;
}

void Hail::ImGuiMaterialEditor::RenderImGuiCommands(ImGuiFileBrowser* fileBrowser, ResourceManager& resourceManager, ImGuiContext* contextObject, bool* closeButton)
{
	InitCommonData();
	ImGui::Begin("Material Editor", closeButton);
	ImGui::Text("Material Name: %s", m_materialToEdit.m_fileObject->m_fileObject.Name().CharString());

	if (m_openTextureBrowser)
	{
		bool finishedBrowsing = false;
		fileBrowser->RenderImGuiCommands(finishedBrowsing);
		if (finishedBrowsing)
		{
			MetaResource textureMetaData;
			resourceManager.GetTextureManager()->LoadTextureMetaData(g_textureFileBrowserData.objectsToSelect[0], textureMetaData);
			m_materialToEdit.m_materialObject.m_textureHandles[m_selectedTextureSlot] = textureMetaData.GetGUID();
			m_selectedTextureSlot = 0;
			m_openTextureBrowser = false;
		}
	}

	if (m_materialToEdit.m_materialObject.m_baseMaterialType == MATERIAL_TYPE::COUNT)
	{
		ImGui::End();
		return;
	}
	if (ImGui::BeginCombo("Material Base Type", ImGuiHelpers::GetMaterialTypeStringFromEnum(m_materialToEdit.m_materialObject.m_baseMaterialType)))
	{
		bool is_selected = (MATERIAL_TYPE::SPRITE == m_materialToEdit.m_materialObject.m_baseMaterialType);
		if (ImGui::Selectable(ImGuiHelpers::GetMaterialTypeStringFromEnum(MATERIAL_TYPE::SPRITE), is_selected))
		{
			m_materialToEdit.m_materialObject.m_baseMaterialType = MATERIAL_TYPE::SPRITE;
		}
		if (is_selected)
			ImGui::SetItemDefaultFocus();
		//
		is_selected = (MATERIAL_TYPE::MODEL3D == m_materialToEdit.m_materialObject.m_baseMaterialType);
		if (ImGui::Selectable(ImGuiHelpers::GetMaterialTypeStringFromEnum(MATERIAL_TYPE::MODEL3D), is_selected))
		{
			m_materialToEdit.m_materialObject.m_baseMaterialType = MATERIAL_TYPE::MODEL3D;
		}
		if (is_selected)
			ImGui::SetItemDefaultFocus();

		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("Blend Mode", ImGuiHelpers::GetMaterialBlendModeFromEnum(m_materialToEdit.m_materialObject.m_blendMode)))
	{

		for (uint8 i = 0; i < (uint8)BLEND_MODE::COUNT; i++)
		{
			bool is_selected = (BLEND_MODE)i == m_materialToEdit.m_materialObject.m_blendMode;
			if (ImGui::Selectable(ImGuiHelpers::GetMaterialBlendModeFromEnum((BLEND_MODE)i), is_selected))
			{
				m_materialToEdit.m_materialObject.m_blendMode = (BLEND_MODE)i;
				is_selected = true;
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	if (MATERIAL_TYPE::SPRITE == m_materialToEdit.m_materialObject.m_baseMaterialType)
	{
		ImGui::Text("Sprite Texture: ");
		ImGui::SameLine();
		if (m_materialToEdit.m_materialObject.m_textureHandles[0] == guidZero)
			ImGui::Text("None");
		else
			ImGui::Text(GetResourceRegistry().GetProjectPath(ResourceType::Texture, m_materialToEdit.m_materialObject.m_textureHandles[0]).Object().Name().CharString());
		ImGui::SameLine();
		if (ImGui::Button("Search") && fileBrowser->Init(&g_textureFileBrowserData))
		{
			m_openTextureBrowser = true;
			m_selectedTextureSlot = 0;
		}
	}

	if (ImGui::Button("Save Material"))
	{
		resourceManager.GetMaterialManager()->ExportMaterial(m_materialToEdit);
		if (contextObject->GetCurrentContextObject() && contextObject->GetCurrentContextType() == ImGuiContextsType::Material)
		{
			MaterialResourceContextObject& material = *(MaterialResourceContextObject*)contextObject->GetCurrentContextObject();
			if (material.m_metaResource.GetGUID() == m_materialToEdit.m_metaResource.GetGUID())
				contextObject->SetCurrentContextObject(ImGuiContextsType::Material,&m_materialToEdit);
		}
	}
	//TODO: reload to original state
	//ImGui::SameLine();
	//if (ImGui::Button("Reload Material"))
	//{
	//
	//}

	ImGui::End();
}

void Hail::ImGuiMaterialEditor::SetMaterialAsset(ImGuiContext* context)
{
	if (context->GetCurrentContextType() != ImGuiContextsType::Material || !context->GetCurrentContextObject())
		return;

	m_materialToEdit = *(MaterialResourceContextObject*)context->GetCurrentContextObject();

}

