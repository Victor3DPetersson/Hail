#include "Engine_PCH.h"
#include "TextureCommons.h"

namespace Hail
{
	void DeleteCompiledTexture(CompiledTexture& texture)
	{
		if (texture.loadState == TEXTURE_LOADSTATE::UNLOADED)
		{
			return;
		}
		SAFEDELETE_ARRAY(texture.compiledColorValues);
		texture.loadState = TEXTURE_LOADSTATE::UNLOADED;
	}

	uint32_t GetTextureByteSize(TextureHeader header)
	{
		uint32_t width, heigth, numberOfColors, byteSizePixel;
		width = header.width;
		heigth = header.height;
		numberOfColors = 0;
		byteSizePixel = 0;

		switch (ToEnum<TEXTURE_TYPE>(header.textureType))
		{
		case TEXTURE_TYPE::R8G8B8A8:
		{
			byteSizePixel = 1;
			numberOfColors = 4;
		}
		break;
		case TEXTURE_TYPE::R8G8B8:
		{
			byteSizePixel = 1;
			numberOfColors = 3;
		}
		break;
		case TEXTURE_TYPE::R8:
		{
			byteSizePixel = 1;
			numberOfColors = 1;
		}
		break;
		case TEXTURE_TYPE::R16G16B16A16:
		{
			byteSizePixel = 2;
			numberOfColors = 4;
		}
		break;
		case TEXTURE_TYPE::R16G16B16:
		{
			byteSizePixel = 2;
			numberOfColors = 3;
		}
		break;
		case TEXTURE_TYPE::R16:
		{
			byteSizePixel = 2;
			numberOfColors = 1;
		}
		break;
		case TEXTURE_TYPE::R32G32B32A32:
		{
			byteSizePixel = 4;
			numberOfColors = 4;
		}
		break;
		case TEXTURE_TYPE::R32G32B32:
		{
			byteSizePixel = 4;
			numberOfColors = 3;
		}
		break;
		case TEXTURE_TYPE::R32:
		{
			byteSizePixel = 4;
			numberOfColors = 1;
		}
		break;
		default:
			break;
		}
		return byteSizePixel * numberOfColors * width * heigth;;
	}
}