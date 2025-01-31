#pragma once
#include "ImGuiFileBrowser.h"
#include "ImGuiContext.h"

namespace Hail
{
	class ImGuiFileBrowser;
	class ResourceManager;
	class ImGuiTextureResource;
	class RenderContext;

	constexpr uint32 MAX_RESOURCE_FOLDER_DEPTH = 8;
	class ImGuiAssetBrowser
	{
	public:
		ImGuiAssetBrowser();
		void DeInit();
		void RenderImGuiCommands(RenderContext* pRenderContext, ImGuiFileBrowser* fileBrowser, ResourceManager* resourceManager, ImGuiContext* contextObject);

	private:
		void InitFileBrowser();
		void InitCommonData(RenderContext* pRenderContext);

		void InitFolder(RenderContext* pRenderContext, const FileObject& fileObject);
		void ImportTextureLogic();
		void ImportShaderResourceLogic();
		void DeselectAllObjects(GrowingArray<SelectAbleFileObject>* pCurrentDirectory);

		bool m_fileBrowsersAreInited = false;
		bool m_openAssetBrowser = false;
		bool m_inited = false;
		ImGuiFileBrowserData m_textureFileBrowserData;
		ImGuiFileBrowserData m_shaderFileBrowserData;
		FileSystem m_fileSystem;

		FileObject m_currentFileDirectoryOpened;

		struct TextureFolder
		{
			GrowingArray<TextureContextAsset> folderTextures;
			FileObject owningFileObject;
		};

		GrowingArray<TextureContextAsset> m_selectedTextureAssets;
		MaterialResourceContextObject m_currentlySelectedMaterialResource;
		ShaderResourceContextObject m_currentlySelectedShaderResource;

		TextureContextAsset m_folderTexture;
		TextureContextAsset m_materialIconTexture;

		StaticArray<GrowingArray<TextureFolder>, MAX_RESOURCE_FOLDER_DEPTH> m_ImGuiTextureResources;
		ResourceManager* m_resourceManager;
		bool m_openedFileBrowser;
		bool m_creatingMaterial;
		String64 m_createResourceName;
		uint32 m_createMaterialType;
	};




}