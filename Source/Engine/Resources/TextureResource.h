#pragma once
#include "../EngineConstants.h"
#include "TextureCommons.h"

namespace Hail
{
	class TextureResource
	{
	public:


	//private:
		String64 textureName;
		uint32_t index = 0;
		GUID m_uuid;
		CompiledTexture m_compiledTextureData;
	};
}

