#pragma once
#include "String.hpp"
#include "TextureCommons.h"
constexpr uint32_t REQUIRED_TEXTURE_COUNT = 1;


class TextureManager
{
public:
	TextureManager();
	void Update();

	bool LoadTexture( const char* textureName, CompiledTexture& outTexture);
	bool LoadAllRequiredTextures();
	//Will return null in return value if shaders are not initialized
	GrowingArray<CompiledTexture>* GetRequiredTextures();

protected:
	bool CompileRequiredTextures();
	void Cleanup();

	GrowingArray<CompiledTexture> m_compiledRequiredTextures;
};