#include "Engine_PCH.h"
#include "ImGuiPropertyWindow.h"

#include "imgui.h"
#include "ImGuiContext.h"

#include "HailEngine.h"
#include "Resources\ResourceRegistry.h"

#include "Utility\StringUtility.h"

#include "Resources\TextureManager.h"
#include "Resources\ResourceManager.h"
#include "Resources\MaterialManager.h"
#include "ImGuiHelpers.h"


using namespace Hail;

namespace
{
	bool LocalDoesTypeRequire2Shaders(eMaterialType materialType)
	{
		switch (materialType)
		{
		case eMaterialType::SPRITE:
		case eMaterialType::MODEL3D:
		case eMaterialType::FULLSCREEN_PRESENT_LETTERBOX:
			return true;
		case eMaterialType::COUNT:
			break;
		default:
			break;
		}
	}

	void RenderShaderProperties(CompiledShader* pShader)
	{
		ImGui::Separator();
		ImGui::Text("Shader Name: %s", pShader->shaderName.Data());
		ImGui::Text("Shader Type: %s", ImGuiHelpers::GetShaderTypeFromEnum((eShaderType)pShader->header.shaderType));
	}

	void RenderTextureProperty(Hail::ImGuiContext* context)
	{
		ImGui::Text("Texture Asset:");
		ImGui::Separator();
		if (!context->GetCurrentContextObject())
			return;

		TextureContextAsset& texture = *(TextureContextAsset*)context->GetCurrentContextObject();

		ImGui::Text("Name: %s", texture.m_fileObject.m_fileObject.Name().CharString());
		ImGui::Text("Texture Format: %s", GetSerializeableTextureTypeAsText((eTextureSerializeableType)texture.m_TextureProperties.textureType));
		ImGui::Text("Width: %i Height: %i", texture.m_TextureProperties.width, texture.m_TextureProperties.height);
	}

	ImGuiPropertyWindowReturnValue RenderMaterialProperty(Hail::ImGuiContext* context)
	{
		MaterialResourceContextObject& material = *(MaterialResourceContextObject*)context->GetCurrentContextObject();
		ImGui::Text("Material Asset\nName: %s", material.m_fileObject->m_fileObject.Name().CharString());

		ImGui::Text("Material Type: %s", ImGuiHelpers::GetMaterialTypeStringFromEnum(material.m_materialObject.m_baseMaterialType));
		ImGui::Text("Blend Mode: %s", ImGuiHelpers::GetMaterialBlendModeFromEnum(material.m_materialObject.m_blendMode));

		MaterialManager* pMatManager = context->GetResourceManager()->GetMaterialManager();
		const uint32 numberOfShaders = LocalDoesTypeRequire2Shaders(material.m_materialObject.m_baseMaterialType) ? 2 : 1;
		for (size_t iShader = 0; iShader < numberOfShaders; iShader++)
		{
			if (material.m_materialObject.m_shaders[iShader].m_id == GuidZero)
			{
				RenderShaderProperties(pMatManager->GetDefaultCompiledLoadedShader(material.m_materialObject.m_shaders[iShader].m_type));
			}
			else
			{
				CompiledShader* pShader = pMatManager->GetCompiledLoadedShader(material.m_materialObject.m_shaders[iShader].m_id);
				if (!pShader)
				{
					pShader = pMatManager->LoadShaderResource(material.m_materialObject.m_shaders[iShader].m_id);
					if (!pShader)
						pShader = pMatManager->GetDefaultCompiledLoadedShader(material.m_materialObject.m_shaders[iShader].m_type);
				}
				RenderShaderProperties(pShader);
			}
		}
		ImGui::Separator();
		for (size_t i = 0; i < MAX_TEXTURE_HANDLES; i++)
		{
			ImGui::Text("Texture slot %i:", i + 1);
			ImGui::SameLine();
			if (material.m_materialObject.m_textureHandles[i] == GuidZero)
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

	ImGuiPropertyWindowReturnValue RenderShaderProperty(Hail::ImGuiContext* context)
	{
		ShaderResourceContextObject& shader = *(ShaderResourceContextObject*)context->GetCurrentContextObject();
		ImGui::Text("Shader Asset\nName: %s", shader.m_pFileObject->m_fileObject.Name().CharString());

		ImGui::Text("Shader Type: %s", ImGuiHelpers::GetShaderTypeFromEnum((eShaderType)shader.m_pShader->header.shaderType));

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
	if (context->GetCurrentContextType() == ImGuiContextsType::Shader)
		RenderShaderProperty(context);

	ImGui::EndChild();
	return ImGuiPropertyWindowReturnValue::NoOp;
}
