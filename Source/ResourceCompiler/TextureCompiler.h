//Interface for the entire engine
#pragma once

#include "TextureCommons.h"
#include "Containers\GrowingArray\GrowingArray.h"

class TextureManager;
namespace TextureCompiler
{
	void CompileAllTextures();
	bool CompileAndExportAllRequiredTextures(const char** requiredTextures, uint32_t numberOfRequiredTextures);
	bool CompileInternalTexture(Hail::TextureHeader header, const char* textureName);
	bool CompileSpecificTGATexture(const char* path, const char* textureName);
	bool IsTextureCompiled(const char* relativePath, const char* textureName);
	void Init();
};