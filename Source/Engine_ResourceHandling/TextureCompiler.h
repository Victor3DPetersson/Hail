#pragma once

#include "Resources_Textures\TextureCommons.h"
#include "Utility\FilePath.hpp"

namespace Hail
{

	namespace TextureCompiler
	{
		// returns the out texture path from the compiled resource
		FilePath CompileSpecificTGATexture(const FilePath& filePath);
	};
}
