#pragma once
#include "String.hpp"
#include "TextureResource.h"
constexpr uint32_t REQUIRED_TEXTURE_COUNT = 3;

namespace Hail
{
	class ResourceManager;

	class TextureManager
	{
		friend class ResourceManager;
	public:
		TextureManager();
		void Update();

		bool LoadTexture(const char* textureName);
		bool LoadAllRequiredTextures();
		//Will return null in return value if textures are not initialized
		GrowingArray<TextureResource>* GetTextures();


	protected:
		bool CompileTexture(const char* textureName);
		void Cleanup();

		GrowingArray<TextureResource> m_loadedTextures;
	};
}
