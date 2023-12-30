#pragma once
#include "../EngineConstants.h"
#include "ImGuiFileBrowser.h"

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
		void RenderImGuiCommands(ImGuiFileBrowser* fileBrowser, ResourceManager* resourceManager);

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

		struct TextureAsset
		{
			ImGuiTextureResource* texture;
			FileObject fileObject;
		};
		FileObject m_currentFileDirectoryOpened;

		struct TextureFolder
		{
			GrowingArray<TextureAsset> folderTextures;
			FileObject owningFileObject;
		};

		StaticArray<GrowingArray<TextureFolder>, MAX_RESOURCE_FOLDER_DEPTH> m_ImGuiTextureResources;
		ResourceManager* m_resourceManager;
		bool m_openedFileBrowser;
	};




}