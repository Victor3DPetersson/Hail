#pragma once
#include "ImGuiFileBrowser.h"
#include "ImGuiContext.h"

namespace Hail
{
	class ImGuiFileBrowser;
	class ResourceManager;
	class ImGuiTextureResource;

	constexpr uint32 MAX_RESOURCE_FOLDER_DEPTH = 8;
	class ImGuiAssetBrowser
	{
	public:
		ImGuiAssetBrowser();
		void RenderImGuiCommands(ImGuiFileBrowser* fileBrowser, ResourceManager* resourceManager, ImGuiContext* contextObject);

	private:
		void InitFileBrowser();
		void InitCommonData();

		void InitFolder(const FileObject& fileObject);
		void ImportTextureLogic();


		bool m_fileBrowsersAreInited = false;
		bool m_openAssetBrowser = false;
		bool m_inited = false;
		ImGuiFileBrowserData m_textureFileBrowserData;
		FileSystem m_fileSystem;

		FileObject m_currentFileDirectoryOpened;

		struct TextureFolder
		{
			GrowingArray<TextureContextAsset> folderTextures;
			FileObject owningFileObject;
		};

		GrowingArray<TextureContextAsset> m_selectedTextureAssets;
		MaterialResourceContextObject m_currentlySelectedMaterialResource;


		TextureContextAsset m_folderTexture;
		TextureContextAsset m_materialIconTexture;

		StaticArray<GrowingArray<TextureFolder>, MAX_RESOURCE_FOLDER_DEPTH> m_ImGuiTextureResources;
		ResourceManager* m_resourceManager;
		bool m_openedFileBrowser;
		bool m_creatingMaterial;
		String256 m_createResourceName;
	};




}