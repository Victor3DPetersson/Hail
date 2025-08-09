#pragma once

#include "Resources_Textures\TextureCommons.h"
#include "Utility\FilePath.hpp"

namespace Hail
{

	namespace TextureCompiler
	{
		// returns the out texture path from the compiled resource, if the GUID is empty a new GUID will be constructed
		FilePath CompileSpecificTGATexture(const FilePath& filePath, GUID guid);
	};
}
