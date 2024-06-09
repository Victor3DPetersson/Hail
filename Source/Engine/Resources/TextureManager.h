#pragma once
#include "TextureResource.h"
#include "Resources_Materials\ShaderTextureList.h"
#include "Resources_Materials\\Materials_Common.h"

namespace Hail
{
	class ResourceManager;
	class RenderingResourceManager;
	class FrameBufferTexture;

	class FilePath;

	constexpr uint32 INVALID_TEXTURE_HANDLE = MAX_UINT;

	class TextureManager
	{
		friend class ResourceManager;
	public:
		virtual void Init(RenderingDevice* device);
		void ClearAllResources();
		void ReloadAllTextures(uint32 frameInFlight);
		void Update();

		bool LoadTexture(const char* textureName);
		// returns the index of the texture in the managed memory
		uint32 LoadTexture(GUID textureID);
		virtual FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) = 0;

		TextureResource* GetTexture(uint32_t index);
		TextureResource* GetEngineTexture(eDecorationSets setDomainToGet, uint32_t bindingIndex, uint32 frameInFlight);
		void RegisterEngineTexture(TextureResource* textureToSet, eDecorationSets setDomain, uint32 bindingIndex, uint32 frameInFlight);
		const TextureResource* GetDefaultTexture() const { return m_defaultTexture; }

		const GrowingArray<TextureResource*>& GetLoadedTextures() const { return m_loadedTextures; }

		//Editor / non game functionality
		FilePath ImportTextureResource(const FilePath& filepath) const;
		virtual ImGuiTextureResource* CreateImGuiTextureResource(const FilePath& filepath, RenderingResourceManager* renderingResourceManager, TextureHeader* headerToFill) = 0;
		virtual void DeleteImGuiTextureResource(ImGuiTextureResource*) = 0;

		static void LoadTextureMetaData(const FilePath& filePath, MetaResource& metaResourceToFill);

	protected:
		void CreateDefaultTexture();
		virtual TextureResource* CreateTextureInternal(const char* name, CompiledTexture& compiledTextureData) = 0;
		virtual void ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight);
		//Reloading will always call ClearTextureInternal
		virtual bool ReloadTextureInternal(int textureIndex, uint32 frameInFlight);
		CompiledTexture LoadTextureInternal(const char* textureName, MetaResource& metaResourceToFill, bool reloadTexture);
		TextureResource* LoadTextureInternalPath(const FilePath& path);
		bool ReadStreamInternal(CompiledTexture& textureToFill, InOutStream& inStream, MetaResource& metaResourceToFill) const;
		bool CompileTexture(const char* textureName);

		RenderingDevice* m_device;
		TextureResource* m_defaultTexture;
		GrowingArray<TextureResource*> m_loadedTextures;

		// Static engine defined textures for material types and global domains
		//StaticArray<TextureResource*, (uint32)eGlobalTextures::count> m_globalTextures;
		StaticArray< StaticArray<TextureResource*, MAX_FRAMESINFLIGHT>, (uint32)eMaterialTextures::count> m_materialTextures;

	};
}
