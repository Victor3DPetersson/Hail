#pragma once
#include "String.hpp"
#include "TextureResource.h"
constexpr uint32_t REQUIRED_TEXTURE_COUNT = 3;

namespace Hail
{
	class ResourceManager;
	class RenderingResourceManager;
	class FrameBufferTexture;

	class FilePath;
	class ImGuiTextureResource;


	class TextureManager
	{
		friend class ResourceManager;
	public:
		virtual void Init(RenderingDevice* device);
		virtual void ClearAllResources() = 0;
		void ReloadAllTextures(uint32 frameInFlight);
		void Update();

		bool LoadTexture(const char* textureName);
		bool LoadAllRequiredTextures();
		virtual FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) = 0;

		const GrowingArray<ResourceValidator>& GetTextureValidators() const { return m_textureCommonDataValidators; }
		const GrowingArray<TextureResource>& GetTexturesCommonData() const { return m_textureCommonData; }
		const TextureResource& GetDefaultTextureCommonData() const { return m_defaultTexture; }

		//Editor / non game functionality
		void ImportTextureResource(const FilePath& filepath) const;
		virtual ImGuiTextureResource* CreateImGuiTextureResource(const FilePath& filepath, RenderingResourceManager* renderingResourceManager) = 0;

	protected:
		void CreateDefaultTexture();
		virtual bool CreateTextureInternal(TextureResource& textureToCreate, bool createDefaultTexture) = 0;
		virtual void ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight);
		//Reloading will always call ClearTextureInternal
		virtual bool ReloadTextureInternal(int textureIndex, uint32 frameInFlight);
		bool LoadTextureInternal(const char* textureName, TextureResource& textureToFill, bool reloadTexture);
		bool CompileTexture(const char* textureName);

		TextureResource m_defaultTexture;

		GrowingArray<TextureResource> m_textureCommonData;
		GrowingArray<ResourceValidator> m_textureCommonDataValidators;



	};
}
