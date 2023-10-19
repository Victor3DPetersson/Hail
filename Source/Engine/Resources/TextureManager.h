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
		void ReloadAllTextures(uint32 frameInFlight);
		void Update();

		virtual bool LoadTexture(const char* textureName) = 0;
		bool LoadAllRequiredTextures();
		virtual FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) = 0;

		const GrowingArray<TextureResource>& GetTexturesCommonData() const { return m_textureCommonData; }

	protected:
		virtual void ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight);
		//Reloading will always call ClearTextureInternal
		virtual bool ReloadTextureInternal(int textureIndex, uint32 frameInFlight);
		bool LoadTextureInternal(const char* textureName, TextureResource& textureToFill, bool reloadTexture);
		bool CompileTexture(const char* textureName);

		GrowingArray<TextureResource> m_textureCommonData;
		GrowingArray<ResourceValidator> m_textureCommonDataValidators;
	};
}
