#include "Engine_PCH.h"
#include "ImGuiPropertyWindow.h"

#include "imgui.h"
#include "ImGuiContext.h"

#include "HailEngine.h"
#include "Resources\ResourceRegistry.h"

#include "Utility\StringUtility.h"

#include "Resources\TextureManager.h"
#include "Resources\ResourceManager.h"
#include "ImGuiHelpers.h"


using namespace Hail;

namespace
{



	void RenderTextureProperty(Hail::ImGuiContext* context)
	{
		ImGui::Text("Texture Asset:");
		ImGui::Separator();
		if (!context->GetCurrentContextObject())
			return;

		TextureContextAsset& texture = *(TextureContextAsset*)context->GetCurrentContextObject();

		ImGui::Text("Name: %s", texture.m_fileObject.m_fileObject.Name().CharString());
		ImGui::Text("Texture Format: %s", GetTextureTypeAsText((TEXTURE_TYPE)texture.m_textureHeader.textureType));
		ImGui::Text("Width: %i Height: %i", texture.m_textureHeader.width, texture.m_textureHeader.height);
	}

	ImGuiPropertyWindowReturnValue RenderMaterialProperty(Hail::ImGuiContext* context)
	{
		MaterialResourceContextObject& material = *(MaterialResourceContextObject*)context->GetCurrentContextObject();
		ImGui::Text("Material Asset\nName: %s", material.m_fileObject->m_fileObject.Name().CharString());

		ImGui::Text("Material Type: %s", ImGuiHelpers::GetMaterialTypeStringFromEnum(material.m_materialObject.m_baseMaterialType));
		ImGui::Text("Blend Mode: %s", ImGuiHelpers::GetMaterialBlendModeFromEnum(material.m_materialObject.m_blendMode));

		for (size_t i = 0; i < MAX_TEXTURE_HANDLES; i++)
		{
			ImGui::Text("Texture slot %i:", i + 1);
			ImGui::SameLine();
			if (material.m_materialObject.m_textureHandles[i] == guidZero)
			{
				ImGui::Text("No texture");
				continue;
			}
			const FilePath texturePath = GetResourceRegistry().GetProjectPath(ResourceType::Texture, material.m_materialObject.m_textureHandles[i]);
			if (!texturePath.IsValid())
				continue;
			ImGui::Text("%s", texturePath.Object().Name().CharString());
		}
		ImGui::Separator();
		if (ImGui::Button("Edit Material"))
			return ImGuiPropertyWindowReturnValue::OpenMaterialWindow;

		return ImGuiPropertyWindowReturnValue::NoOp;
	}
}


ImGuiPropertyWindowReturnValue ImGuiPropertyWindow::RenderImGuiCommands(ImGuiFileBrowser* fileBrowser, ImGuiContext* context)
{
	ImGui::BeginChild("Properties", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_MenuBar);

	if (context->GetCurrentContextType() == ImGuiContextsType::None)
	{
		ImGui::Text("No item selected");
		ImGui::EndChild();
		return ImGuiPropertyWindowReturnValue::NoOp;
	}

	if (context->GetCurrentContextType() == ImGuiContextsType::Texture)
		RenderTextureProperty(context);

	if (context->GetCurrentContextType() == ImGuiContextsType::Material)
	{
		if (RenderMaterialProperty(context) == ImGuiPropertyWindowReturnValue::OpenMaterialWindow)
		{
			ImGui::EndChild();
			return ImGuiPropertyWindowReturnValue::OpenMaterialWindow;
		}
	}

	ImGui::EndChild();
	return ImGuiPropertyWindowReturnValue::NoOp;
}
