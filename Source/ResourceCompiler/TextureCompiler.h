//Interface for the entire engine
#pragma once

#include "TextureCommons.h"


namespace Hail
{
	class FilePath;

	namespace TextureCompiler
	{
		void CompileAllTextures();
		bool CompileAndExportAllRequiredTextures(const char** requiredTextures, uint32 numberOfRequiredTextures);
		bool CompileInternalTexture(TextureHeader header, const char* textureName);
		bool CompileSpecificTGATexture(const FilePath& filePath);
		bool IsTextureCompiled(const char* relativePath, const char* textureName);
		void Init();
	};
}
