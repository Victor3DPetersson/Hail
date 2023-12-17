#include "Engine_PCH.h"
#include "ImGuiAssetBrowser.h"


#include "imgui.h"

#include "MathUtils.h"
#include "Resources\MaterialResources.h"
#include "Utility\StringUtility.h"

#include "Resources\TextureManager.h"
#include "Resources\ResourceManager.h"
#include "ImGuiDirectoryPanel.h"
#ifdef PLATFORM_WINDOWS
#include "Resources\Vulkan\VlkTextureResource.h"
#endif

using namespace Hail;

namespace 
{
	//StaticArray<String64, (uint32_t)MATERIAL_TYPE::COUNT> g_materialTypeStrings;
	constexpr uint32 ASSET_SIZE = 100;

	void RenderAssetPreview(FileObject& fileObject, void* imageResource)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::BeginChild(fileObject.Name().CharString().Data(), { ASSET_SIZE, ASSET_SIZE * 1.3 });
		ImGuiID id = ImGui::GetID(fileObject.Name().CharString().Data());

		io.MousePos;
		const ImVec2 windowPos = ImGui::GetWindowPos();
		const bool hovered = 
			windowPos.x < io.MousePos.x 
			&& windowPos.x + ASSET_SIZE > io.MousePos.x
			&& windowPos.y < io.MousePos.y 
			&& windowPos.y + ASSET_SIZE * 1.3 > io.MousePos.y;

		if (hovered)
			ImGui::BeginChildFrame(id, { ASSET_SIZE, ASSET_SIZE * 1.3 });
		else
			ImGui::BeginChildFrame(id, { ASSET_SIZE, ASSET_SIZE * 1.3 }, ImGuiWindowFlags_NoBackground);
		
		ImVec2 uv_min = ImVec2(0.0f, 1.0f);                 // Top-left
		ImVec2 uv_max = ImVec2(1.0f, 0.0f);                 // Lower-right
		ImVec4 tint_col = ImGui::GetStyleColorVec4(ImGuiCol_Text);   // No tint
		ImVec4 border_col = ImVec4(1.0, 1.0, 1.0, 0.10);
		ImGui::Image(*(VkDescriptorSet*)imageResource, ImVec2(ASSET_SIZE * 0.9, ASSET_SIZE * 0.9), uv_min, uv_max, tint_col, border_col);

		ImGui::TextWrapped(fileObject.Name().CharString().Data());
		ImGui::EndChildFrame();
		ImGui::EndChild();
	}

}


Hail::ImGuiAssetBrowser::ImGuiAssetBrowser()
{
	m_openedFileBrowser = false;
	m_resourceManager = nullptr;
}

void ImGuiAssetBrowser::RenderImGuiCommands(ImGuiFileBrowser* fileBrowser, ResourceManager* resourceManager)
{
	m_resourceManager = resourceManager;
	InitCommonData();
	if (m_openedFileBrowser)
	{
		bool closeCommandWasSent = false;
		fileBrowser->RenderImGuiCommands(closeCommandWasSent);
		if (closeCommandWasSent)
		{
			ImportTextureLogic();
			m_openedFileBrowser = false;
		}
	}
	ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_MenuBar);

	// Menu Bar
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Import Texture", NULL, &m_openedFileBrowser))
				fileBrowser->Init(&m_textureFileBrowserData);
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}


	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y * 0.9f), false, window_flags);
	}

	ImGui::BeginDisabled();
	ImGui::Button("Create Folder");
	ImGui::EndDisabled();

	if (ImGuiHelpers::DirectoryPanelLogic(&m_fileSystem, m_fileSystem.GetBaseDepth()))
	{
		m_currentFileDirectoryOpened = m_fileSystem.GetCurrentFileDirectoryObject();
		InitFolder(m_fileSystem.GetCurrentFileDirectoryObject());
	}

	ImGui::EndChild();
	ImGui::SameLine();


	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("ChildR", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.9f), true, window_flags);
	}

	GrowingArray<TextureFolder>& texturesLoadedInFolder = m_ImGuiTextureResources[m_currentFileDirectoryOpened.GetDirectoryLevel() - m_fileSystem.GetBaseDepth()];
	for (size_t i = 0; i < texturesLoadedInFolder.Size(); i++)
	{
		TextureFolder& folder = texturesLoadedInFolder[i];
		if (m_currentFileDirectoryOpened == folder.owningFileObject)
		{
			for (size_t iTexture = 0; iTexture < folder.folderTextures.Size(); iTexture++)
			{
				RenderAssetPreview(folder.folderTextures[iTexture].fileObject, folder.folderTextures[iTexture].texture->GetImguiTextureResource());
				if (iTexture + 1 % 6 != 0 && iTexture != folder.folderTextures.Size() - 1)
				{
					ImGui::SameLine();
				}
			}
			break;
		}
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();



	ImGui::End();

}

void Hail::ImGuiAssetBrowser::InitFileBrowser()
{
	if (m_fileBrowsersAreInited)
	{
		return;
	}
	m_fileBrowsersAreInited = true;
	m_textureFileBrowserData.allowMultipleSelection = true;
	m_textureFileBrowserData.objectsToSelect.Init(16);
	m_textureFileBrowserData.extensionsToSearchFor = { "tga" };
	m_textureFileBrowserData.pathToBeginSearchingIn = RESOURCE_DIR;
}

void Hail::ImGuiAssetBrowser::InitCommonData()
{
	if (m_inited)
	{
		return;
	}
	m_inited = true;
	
	m_fileSystem.SetFilePathAndInit(RESOURCE_DIR_OUT, { "txr" });
	m_currentFileDirectoryOpened = m_fileSystem.GetCurrentFileDirectoryObject();
	InitFileBrowser();
	InitFolder(m_fileSystem.GetCurrentFileDirectoryObject());
}

void Hail::ImGuiAssetBrowser::InitFolder(const FileObject& fileObject)
{
	const GrowingArray<SelecteableFileObject>* directory = m_fileSystem.GetFileDirectory(fileObject);
	if (!directory)
		return;

	const uint16 directoryLevel = fileObject.GetDirectoryLevel() - m_fileSystem.GetBaseDepth();


	GrowingArray<TextureFolder>& foldersAtLevel = m_ImGuiTextureResources[directoryLevel];

	if (!foldersAtLevel.IsInitialized())
	{
		foldersAtLevel.Init(4);
	}
	else
	{
		//check if folder already has been initialized and created
		for (size_t i = 0; i < foldersAtLevel.Size(); i++)
		{
			if (foldersAtLevel[i].owningFileObject == fileObject)
				return;
		}
	}

	TextureFolder textureFolder;
	textureFolder.owningFileObject = fileObject;
	textureFolder.folderTextures.Init(directory[0].Size());
	for (size_t fileI = 0; fileI < directory[0].Size(); fileI++)
	{
		const SelecteableFileObject& fileObject = directory[0][fileI];

		if (fileObject.m_fileObject.IsFile() && StringCompare(fileObject.m_fileObject.Extension(), L"txr"))
		{
			ImGuiTextureResource* texture = new ImGuiVlkTextureResource();
			memcpy(texture, m_resourceManager->GetTextureManager()->CreateImGuiTextureResource(m_fileSystem.GetCurrentFilePath() + fileObject.m_fileObject, m_resourceManager->GetRenderingResourceManager()), sizeof(ImGuiVlkTextureResource));
			TextureAsset textureAsset;
			textureAsset.texture = texture;
			textureAsset.fileObject = fileObject.m_fileObject;

			textureFolder.folderTextures.Add(textureAsset);
		}
	}
	foldersAtLevel.Add(textureFolder);
}

void Hail::ImGuiAssetBrowser::ImportTextureLogic()
{

}

