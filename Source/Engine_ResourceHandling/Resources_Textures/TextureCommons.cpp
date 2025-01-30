#include "ResourceCompiler_PCH.h"
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

	uint32_t GetTextureByteSize(TextureProperties properties)
	{
		uint32_t width, heigth, numberOfColors, byteSizePixel;
		width = properties.width;
		heigth = properties.height;
		numberOfColors = 0;
		byteSizePixel = 0;

		switch (ToEnum<eTextureSerializeableType>(properties.textureType))
		{
		case eTextureSerializeableType::R32G32B32A32F:
		{
			byteSizePixel = 4;
			numberOfColors = 4;
			break;
		}
		case eTextureSerializeableType::R32G32B32F:
		{
			byteSizePixel = 4;
			numberOfColors = 3;

			break;
		}
		case eTextureSerializeableType::R32F:
		{
			byteSizePixel = 4;
			numberOfColors = 1;
			break;
		}
		case eTextureSerializeableType::R8G8B8A8_SRGB:
		{
			byteSizePixel = 1;
			numberOfColors = 4;
		}
		break;
		case eTextureSerializeableType::R8G8B8_SRGB:
		{
			byteSizePixel = 1;
			numberOfColors = 3;
		}
		break;
		case eTextureSerializeableType::R8G8B8A8:
		{
			byteSizePixel = 1;
			numberOfColors = 4;
		}
		break;
		case eTextureSerializeableType::R8G8B8:
		{
			byteSizePixel = 1;
			numberOfColors = 3;
		}
		break;
		case eTextureSerializeableType::R8:
		{
			byteSizePixel = 1;
			numberOfColors = 1;
		}
		break;
		case eTextureSerializeableType::R16G16B16A16:
		{
			byteSizePixel = 2;
			numberOfColors = 4;
		}
		break;
		case eTextureSerializeableType::R16G16B16:
		{
			byteSizePixel = 2;
			numberOfColors = 3;
		}
		break;
		case eTextureSerializeableType::R16:
		{
			byteSizePixel = 2;
			numberOfColors = 1;
		}
		break;
		case eTextureSerializeableType::R32G32B32A32:
		{
			byteSizePixel = 4;
			numberOfColors = 4;
		}
		break;
		case eTextureSerializeableType::R32G32B32:
		{
			byteSizePixel = 4;
			numberOfColors = 3;
		}
		break;
		case eTextureSerializeableType::R32:
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

	eTextureFormat SerializeableTextureTypeToTextureFormat(eTextureSerializeableType type)
	{
		switch (type)
		{
		case Hail::eTextureSerializeableType::R32G32B32A32F:
			return eTextureFormat::R32G32B32A32_SFLOAT;
			break;
		case Hail::eTextureSerializeableType::R32G32B32F:
			return eTextureFormat::R32G32B32_SFLOAT;
			break;
		case Hail::eTextureSerializeableType::R32F:
			return eTextureFormat::R32_SFLOAT;
			break;
		case Hail::eTextureSerializeableType::R32G32B32A32:
			return eTextureFormat::R32G32B32A32_UINT;
			break;
		case Hail::eTextureSerializeableType::R32G32B32:
			return eTextureFormat::R32G32B32_UINT;
			break;
		case Hail::eTextureSerializeableType::R32:
			return eTextureFormat::R32_UINT;
			break;
		case Hail::eTextureSerializeableType::R16G16B16A16:
			return eTextureFormat::R16G16B16A16_UINT;
			break;
		case Hail::eTextureSerializeableType::R16G16B16:
			return eTextureFormat::R16G16B16_UINT;
			break;
		case Hail::eTextureSerializeableType::R16:
			return eTextureFormat::R16_UINT;
			break;
		case Hail::eTextureSerializeableType::R8G8B8A8:
			return eTextureFormat::R8G8B8A8_UNORM;
			break;
		case Hail::eTextureSerializeableType::R8G8B8:
			return eTextureFormat::R8G8B8A8_UNORM;
			break;
		case Hail::eTextureSerializeableType::R8:
			return eTextureFormat::R8_UNORM;
			break;
		case Hail::eTextureSerializeableType::R8G8B8A8_SRGB:
			return eTextureFormat::R8G8B8A8_SRGB;
			break;
		case Hail::eTextureSerializeableType::R8G8B8_SRGB:
			return eTextureFormat::R8G8B8A8_SRGB;
			break;
		case Hail::eTextureSerializeableType::B8G8R8A8_UNORM:
			return eTextureFormat::B8G8R8A8_UNORM;
			break;
		default:
			return eTextureFormat::UNDEFINED;
			break;
		}
	}

	const char* GetSerializeableTextureTypeAsText(eTextureSerializeableType type)
	{
		switch (type)
		{
		case Hail::eTextureSerializeableType::R32G32B32A32F:
		case Hail::eTextureSerializeableType::R32G32B32F:
		case Hail::eTextureSerializeableType::R32F:
		case Hail::eTextureSerializeableType::R32G32B32A32:
		case Hail::eTextureSerializeableType::R32G32B32:
		case Hail::eTextureSerializeableType::R32:
		case Hail::eTextureSerializeableType::R16G16B16A16:
		case Hail::eTextureSerializeableType::R16G16B16:
		case Hail::eTextureSerializeableType::R16:
		case Hail::eTextureSerializeableType::R8G8B8A8:
		case Hail::eTextureSerializeableType::R8G8B8:
			return "Linear";
		case Hail::eTextureSerializeableType::R8G8B8A8_SRGB:
		case Hail::eTextureSerializeableType::R8G8B8_SRGB:
			return "SRGB";
		default:
			return "";
			break;
		}
		return "";
	}
}