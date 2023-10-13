#pragma once
#include "String.hpp"
#include "TextureResource.h"
constexpr uint32_t REQUIRED_TEXTURE_COUNT = 3;

namespace Hail
{
	class ResourceManager;
	class FrameBufferTexture;

	class TextureManager
	{
		friend class ResourceManager;
	public:
		virtual void Init(RenderingDevice* device);
		virtual void ClearAllResources() = 0;
		//TODO: replace with a guiid instead of a const char*
		void ClearTexture(const char* textureName);
		bool ReloadTexture(const char* textureName);
		void ReloadAllTextures(GrowingArray<String64>& reloadedTextures);
		void Update();

		virtual bool LoadTexture(const char* textureName) = 0;
		bool LoadAllRequiredTextures();
		virtual FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) = 0;

		const GrowingArray<TextureResource>& GetTexturesCommonData() const { return m_textureCommonData; }


	protected:
		virtual void ClearTextureInternal(int textureIndex);
		//Reloading will always call ClearTextureInternal
		virtual bool ReloadTextureInternal(int textureIndex);
		bool LoadTextureInternal(const char* textureName, TextureResource& textureToFill);
		bool CompileTexture(const char* textureName);

		GrowingArray<TextureResource> m_textureCommonData;
	};
}
