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
		case TEXTURE_TYPE::R8G8B8A8_SRGB:
		{
			byteSizePixel = 1;
			numberOfColors = 4;
		}
		break;
		case TEXTURE_TYPE::R8G8B8_SRGB:
		{
			byteSizePixel = 1;
			numberOfColors = 3;
		}
		break;
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
		return byteSizePixel * numberOfColors * width * heigth;
	}
	TEXTURE_FORMAT TextureTypeToTextureFormat(TEXTURE_TYPE type)
	{
		switch (type)
		{
		case Hail::TEXTURE_TYPE::R32G32B32A32F:
			return TEXTURE_FORMAT::R32G32B32A32_SFLOAT;
			break;
		case Hail::TEXTURE_TYPE::R32G32B32F:
			return TEXTURE_FORMAT::R32G32B32_SFLOAT;
			break;
		case Hail::TEXTURE_TYPE::R32F:
			return TEXTURE_FORMAT::R32_SFLOAT;
			break;
		case Hail::TEXTURE_TYPE::R32G32B32A32:
			return TEXTURE_FORMAT::R32G32B32A32_UINT;
			break;
		case Hail::TEXTURE_TYPE::R32G32B32:
			return TEXTURE_FORMAT::R32G32B32_UINT;
			break;
		case Hail::TEXTURE_TYPE::R32:
			return TEXTURE_FORMAT::R32_UINT;
			break;
		case Hail::TEXTURE_TYPE::R16G16B16A16:
			return TEXTURE_FORMAT::R16G16B16A16_UINT;
			break;
		case Hail::TEXTURE_TYPE::R16G16B16:
			return TEXTURE_FORMAT::R16G16B16_UINT;
			break;
		case Hail::TEXTURE_TYPE::R16:
			return TEXTURE_FORMAT::R16_UINT;
			break;
		case Hail::TEXTURE_TYPE::R8G8B8A8:
			return TEXTURE_FORMAT::R8G8B8A8_UNORM;
			break;
		case Hail::TEXTURE_TYPE::R8G8B8:
			return TEXTURE_FORMAT::R8G8B8A8_UNORM;
			break;
		case Hail::TEXTURE_TYPE::R8:
			return TEXTURE_FORMAT::R8_UNORM;
			break;
		case Hail::TEXTURE_TYPE::R8G8B8A8_SRGB:
			return TEXTURE_FORMAT::R8G8B8A8_SRGB;
			break;
		case Hail::TEXTURE_TYPE::R8G8B8_SRGB:
			return TEXTURE_FORMAT::R8G8B8A8_SRGB;
			break;
			break;
		default:
			return TEXTURE_FORMAT::UNDEFINED;
			break;
		}
	}

	const char* GetTextureTypeAsText(TEXTURE_TYPE type)
	{
		switch (type)
		{
		case Hail::TEXTURE_TYPE::R32G32B32A32F:
		case Hail::TEXTURE_TYPE::R32G32B32F:
		case Hail::TEXTURE_TYPE::R32F:
		case Hail::TEXTURE_TYPE::R32G32B32A32:
		case Hail::TEXTURE_TYPE::R32G32B32:
		case Hail::TEXTURE_TYPE::R32:
		case Hail::TEXTURE_TYPE::R16G16B16A16:
		case Hail::TEXTURE_TYPE::R16G16B16:
		case Hail::TEXTURE_TYPE::R16:
		case Hail::TEXTURE_TYPE::R8G8B8A8:
		case Hail::TEXTURE_TYPE::R8G8B8:
			return "Linear";
		case Hail::TEXTURE_TYPE::R8G8B8A8_SRGB:
		case Hail::TEXTURE_TYPE::R8G8B8_SRGB:
			return "SRGB";
		default:
			return "";
			break;
		}
		return "";
	}
}