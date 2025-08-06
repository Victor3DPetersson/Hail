#pragma once
#include "TextureResource.h"
#include "Resources_Materials\ShaderTextureList.h"
#include "Resources_Materials\\Materials_Common.h"

namespace Hail
{
	class RenderContext;
	class ResourceManager;
	class RenderingResourceManager;
	class FrameBufferTexture;

	class FilePath;

	constexpr uint32 INVALID_TEXTURE_HANDLE = MAX_UINT;

	class TextureManager
	{
		friend class ResourceManager;
	public:

		explicit TextureManager(RenderingDevice* pDevice);

		struct TextureWithView
		{
			TextureResource* m_pTexture;
			TextureView* m_pView;
		};

		void Init(RenderContext* pRenderContext);
		void ClearAllResources();
		void ReloadAllTextures(uint32 frameInFlight);
		void Update(RenderContext* pRenderContext);

		bool LoadTexture(const char* textureName);
		// Will return the texture handle ID and deferr the upload to the GPU.
		uint32 StagerredTextureLoad(GUID textureID);

		virtual FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, eTextureFormat format, TEXTURE_DEPTH_FORMAT depthFormat) = 0;
		virtual TextureView* CreateTextureView() = 0;

		TextureResource* GetTexture(uint32_t index);
		TextureResource* CreateTexture(RenderContext* pRenderContext, const char* name, CompiledTexture& compiledTextureData);
		TextureResource* CreateTexture(RenderContext* pRenderContext, const char* name, TextureProperties& textureProperties);
		// Imports the resource if it is not present in the compiled texture folder, used for external textures in rendering systems
		TextureView* CreateTextureView(const RelativeFilePath filepath, RenderContext* pRenderContext);
		// Creates a view from properties, used for internal resources
		TextureView* CreateTextureView(TextureViewProperties& viewProps);
		TextureView* GetTextureView(uint32_t index);
		TextureView* GetEngineTextureView(eDecorationSets setDomainToGet, uint32_t bindingIndex, uint32 frameInFlight);
		void RegisterEngineTexture(TextureResource* pTexture, TextureView* pTextureView, eDecorationSets setDomain, uint32 bindingIndex, uint32 frameInFlight);
		const TextureWithView GetDefaultTexture() const { return m_defaultTexture; }

		const GrowingArray<TextureWithView>& GetLoadedTextures() const { return m_loadedTextures; }

		//Editor / non game functionality
		FilePath ImportTextureResource(const FilePath& filepath) const;
		virtual ImGuiTextureResource* CreateImGuiTextureResource(RenderContext* pRenderContext, const FilePath& filepath, RenderingResourceManager* renderingResourceManager, TextureProperties* headerToFill) = 0;
		virtual void DeleteImGuiTextureResource(ImGuiTextureResource*) = 0;

		static void LoadTextureMetaData(const FilePath& filePath, MetaResource& metaResourceToFill);

		// Creates the texture and its underlying structures
		virtual TextureResource* CreateTextureInternalNoLoad() = 0;
	protected:

		void CreateDefaultTexture(RenderContext* pRenderContext);
		virtual TextureResource* CreateTextureInternal(const char* name, CompiledTexture& compiledTextureData) = 0;

		virtual void ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight);
		//Reloading will always call ClearTextureInternal
		virtual bool ReloadTextureInternal(int textureIndex, uint32 frameInFlight);
		CompiledTexture LoadTextureInternal(const char* textureName, MetaResource& metaResourceToFill, bool reloadTexture);
		//TextureResource* LoadTextureInternalPath(const FilePath& path);
		TextureResource* LoadTextureRequestInternal(const FilePath& path);
		bool ReadStreamInternal(CompiledTexture& textureToFill, InOutStream& inStream, MetaResource& metaResourceToFill) const;
		bool CompileTexture(const char* textureName);

		virtual bool CreateTextureGPUData(RenderContext* pRenderContext, CompiledTexture& compiledTextureData, TextureResource* pTextureResource) = 0;

		RenderingDevice* m_device;
		TextureWithView m_defaultTexture;
		
		// TODO: Make a way to remove loaded textures
		GrowingArray<TextureWithView> m_loadedTextures;
		//GrowingArray<TextureView*> m_pTextureViews;

		// Static engine defined textures for material types and global domains
		//StaticArray<TextureResource*, (uint32)eGlobalTextures::count> m_globalTextures;
		StaticArray< StaticArray<TextureWithView, MAX_FRAMESINFLIGHT>, (uint32)eMaterialTextures::count> m_materialTextures;

	private:

		struct TextureGPULoadRequest
		{
			StringL textureName;
			CompiledTexture compiledTextureData;
			uint32 loadedIndex;
		};

		void ClearTextureGPULoadRequest(TextureGPULoadRequest& requestToClear);

		GrowingArray<TextureGPULoadRequest> m_loadRequests;
	};
}
