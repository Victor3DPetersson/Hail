#pragma once

#include "Resources_Textures\TextureCommons.h"
#include "Utility\FilePath.hpp"

namespace Hail
{

	namespace TextureCompiler
	{
		void CompileAllTextures();
		bool CompileAndExportAllRequiredTextures(const char** requiredTextures, uint32 numberOfRequiredTextures);
		bool CompileInternalTexture(TextureProperties header, const char* textureName);
		// returns the out texture path from the compiled resource
		FilePath CompileSpecificTGATexture(const FilePath& filePath);
		bool IsTextureCompiled(const char* relativePath, const char* textureName);
		void Init();
	};
}
