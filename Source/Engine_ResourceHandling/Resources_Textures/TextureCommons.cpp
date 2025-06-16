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
	bool TextureFormatMatchesDecoration(eTextureFormat textureFormat, const ShaderDecoration& decorationToCheck)
	{
		bool bIsMatching = true;
		switch (textureFormat)
		{
		case Hail::eTextureFormat::UNDEFINED:
			// Unsupported for now, will be matching and checked for when engine support implements it
		case Hail::eTextureFormat::R4G4_UNORM_PACK8:
		case Hail::eTextureFormat::R4G4B4A4_UNORM_PACK16:
		case Hail::eTextureFormat::B4G4R4A4_UNORM_PACK16:
		case Hail::eTextureFormat::R5G6B5_UNORM_PACK16:
		case Hail::eTextureFormat::B5G6R5_UNORM_PACK16:
		case Hail::eTextureFormat::R5G5B5A1_UNORM_PACK16:
		case Hail::eTextureFormat::B5G5R5A1_UNORM_PACK16:
		case Hail::eTextureFormat::A1R5G5B5_UNORM_PACK16:
		case Hail::eTextureFormat::A2R10G10B10_UNORM_PACK32:
		case Hail::eTextureFormat::A2R10G10B10_SNORM_PACK32:
		case Hail::eTextureFormat::A2R10G10B10_USCALED_PACK32:
		case Hail::eTextureFormat::A2R10G10B10_SSCALED_PACK32:
		case Hail::eTextureFormat::A2R10G10B10_UINT_PACK32:
		case Hail::eTextureFormat::A2R10G10B10_SINT_PACK32:
		case Hail::eTextureFormat::A2B10G10R10_UNORM_PACK32:
		case Hail::eTextureFormat::A2B10G10R10_SNORM_PACK32:
		case Hail::eTextureFormat::A2B10G10R10_USCALED_PACK32:
		case Hail::eTextureFormat::A2B10G10R10_SSCALED_PACK32:
		case Hail::eTextureFormat::A2B10G10R10_UINT_PACK32:
		case Hail::eTextureFormat::A2B10G10R10_SINT_PACK32:
		case Hail::eTextureFormat::B10G11R11_UFLOAT_PACK32:
		case Hail::eTextureFormat::E5B9G9R9_UFLOAT_PACK32:
			bIsMatching = false;
			break;
		case Hail::eTextureFormat::R8_UNORM:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::uint8norm;
			break;
		case Hail::eTextureFormat::R8_SNORM:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::int8norm;
			break;
		case Hail::eTextureFormat::R8_UINT:
		case Hail::eTextureFormat::R8_USCALED:
		case Hail::eTextureFormat::R8_SRGB:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::uint8;
			break;
		case Hail::eTextureFormat::R8_SINT:
		case Hail::eTextureFormat::R8_SSCALED:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::int8;
			break;
		case Hail::eTextureFormat::R8G8_USCALED:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::uint8norm;
			break;
		case Hail::eTextureFormat::R8G8_SNORM:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::int8norm;
			break;
			break;
			break;
		case Hail::eTextureFormat::R8G8_UNORM:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::uint8norm;
		case Hail::eTextureFormat::R8G8_UINT:
		case Hail::eTextureFormat::R8G8_SRGB:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::uint8;
			break;
		case Hail::eTextureFormat::R8G8_SSCALED:
		case Hail::eTextureFormat::R8G8_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::int8;
			break;
		case Hail::eTextureFormat::R8G8B8_UNORM:
		case Hail::eTextureFormat::B8G8R8_UNORM:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::uint8norm;
			break;
		case Hail::eTextureFormat::R8G8B8_SNORM:
		case Hail::eTextureFormat::B8G8R8_SNORM:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::int8norm;
			break;
		case Hail::eTextureFormat::R8G8B8_USCALED:
		case Hail::eTextureFormat::R8G8B8_UINT:
		case Hail::eTextureFormat::R8G8B8_SRGB:
		case Hail::eTextureFormat::B8G8R8_USCALED:
		case Hail::eTextureFormat::B8G8R8_UINT:
		case Hail::eTextureFormat::B8G8R8_SRGB:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::uint8;
			break;
		case Hail::eTextureFormat::R8G8B8_SSCALED:
		case Hail::eTextureFormat::R8G8B8_SINT:
		case Hail::eTextureFormat::B8G8R8_SSCALED:
		case Hail::eTextureFormat::B8G8R8_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::int8;
			break;
		case Hail::eTextureFormat::R8G8B8A8_UNORM:
		case Hail::eTextureFormat::B8G8R8A8_UNORM:
		case Hail::eTextureFormat::A8B8G8R8_UNORM_PACK32:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::uint8norm;
			break;
		case Hail::eTextureFormat::R8G8B8A8_SNORM:
		case Hail::eTextureFormat::B8G8R8A8_SNORM:
		case Hail::eTextureFormat::A8B8G8R8_SNORM_PACK32:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::int8norm;
			break;
		case Hail::eTextureFormat::R8G8B8A8_USCALED:
		case Hail::eTextureFormat::R8G8B8A8_UINT:
		case Hail::eTextureFormat::R8G8B8A8_SRGB:
		case Hail::eTextureFormat::B8G8R8A8_USCALED:
		case Hail::eTextureFormat::B8G8R8A8_UINT:
		case Hail::eTextureFormat::B8G8R8A8_SRGB:
		case Hail::eTextureFormat::A8B8G8R8_USCALED_PACK32:
		case Hail::eTextureFormat::A8B8G8R8_UINT_PACK32:
		case Hail::eTextureFormat::A8B8G8R8_SRGB_PACK32:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::uint8;
			break;
		case Hail::eTextureFormat::R8G8B8A8_SSCALED:
		case Hail::eTextureFormat::R8G8B8A8_SINT:
		case Hail::eTextureFormat::B8G8R8A8_SSCALED:
		case Hail::eTextureFormat::B8G8R8A8_SINT:
		case Hail::eTextureFormat::A8B8G8R8_SSCALED_PACK32:
		case Hail::eTextureFormat::A8B8G8R8_SINT_PACK32:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::int8;
			break;
		case Hail::eTextureFormat::R16_UNORM:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::uint16norm;
			break;
		case Hail::eTextureFormat::R16_SNORM:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::int16norm;
			break;
		case Hail::eTextureFormat::R16_USCALED:
		case Hail::eTextureFormat::R16_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::uint16;
			break;
		case Hail::eTextureFormat::R16_SSCALED:
		case Hail::eTextureFormat::R16_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::int16;
			break;
		case Hail::eTextureFormat::R16_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::float16;
			break;
		case Hail::eTextureFormat::R16G16_UNORM:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::uint16norm;
			break;
		case Hail::eTextureFormat::R16G16_SNORM:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::int16norm;
			break;
		case Hail::eTextureFormat::R16G16_USCALED:
		case Hail::eTextureFormat::R16G16_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::uint16;
			break;
		case Hail::eTextureFormat::R16G16_SSCALED:
		case Hail::eTextureFormat::R16G16_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::int16;
			break;
		case Hail::eTextureFormat::R16G16_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::float16;
			break;
		case Hail::eTextureFormat::R16G16B16_UNORM:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::uint16norm;
			break;
		case Hail::eTextureFormat::R16G16B16_SNORM:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::int16norm;
			break;
		case Hail::eTextureFormat::R16G16B16_USCALED:
		case Hail::eTextureFormat::R16G16B16_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::uint16;
			break;
		case Hail::eTextureFormat::R16G16B16_SSCALED:
		case Hail::eTextureFormat::R16G16B16_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::int16;
			break;
		case Hail::eTextureFormat::R16G16B16_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::float16;
			break;
		case Hail::eTextureFormat::R16G16B16A16_UNORM:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::uint16norm;
			break;
		case Hail::eTextureFormat::R16G16B16A16_SNORM:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::int16norm;
			break;
		case Hail::eTextureFormat::R16G16B16A16_USCALED:
		case Hail::eTextureFormat::R16G16B16A16_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::uint16;
			break;
		case Hail::eTextureFormat::R16G16B16A16_SSCALED:
		case Hail::eTextureFormat::R16G16B16A16_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::int16;
			break;
		case Hail::eTextureFormat::R16G16B16A16_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::float16;
			break;
		case Hail::eTextureFormat::R32_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::uint32;
			break;
		case Hail::eTextureFormat::R32_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::int32;
			break;
		case Hail::eTextureFormat::R32_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::float32;
			break;
		case Hail::eTextureFormat::R32G32_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::uint32;
			break;
		case Hail::eTextureFormat::R32G32_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::int32;
			break;
		case Hail::eTextureFormat::R32G32_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::float32;
			break;
		case Hail::eTextureFormat::R32G32B32_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::uint32;
			break;
		case Hail::eTextureFormat::R32G32B32_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::int32;
			break;
		case Hail::eTextureFormat::R32G32B32_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::float32;
			break;
		case Hail::eTextureFormat::R32G32B32A32_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::uint32;
			break;
		case Hail::eTextureFormat::R32G32B32A32_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::int32;
			break;
		case Hail::eTextureFormat::R32G32B32A32_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::float32;
			break;
		case Hail::eTextureFormat::R64_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::uint64;
			break;
		case Hail::eTextureFormat::R64_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::int64;
			break;
		case Hail::eTextureFormat::R64_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 1 && decorationToCheck.m_valueType == eShaderValueType::float64;
			break;
		case Hail::eTextureFormat::R64G64_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::uint64;
			break;
		case Hail::eTextureFormat::R64G64_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::int64;
			break;
		case Hail::eTextureFormat::R64G64_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 2 && decorationToCheck.m_valueType == eShaderValueType::float64;
			break;
		case Hail::eTextureFormat::R64G64B64_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::uint64;
			break;
		case Hail::eTextureFormat::R64G64B64_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::int64;
			break;
		case Hail::eTextureFormat::R64G64B64_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 3 && decorationToCheck.m_valueType == eShaderValueType::float64;
			break;
		case Hail::eTextureFormat::R64G64B64A64_UINT:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::uint64;
			break;
		case Hail::eTextureFormat::R64G64B64A64_SINT:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::int64;
			break;
		case Hail::eTextureFormat::R64G64B64A64_SFLOAT:
			bIsMatching = decorationToCheck.m_elementCount == 4 && decorationToCheck.m_valueType == eShaderValueType::float64;
			break;
		default:
			bIsMatching = false;
			break;
		}
		return bIsMatching;
	}
}