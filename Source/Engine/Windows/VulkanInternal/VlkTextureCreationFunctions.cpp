#include "Engine_PCH.h"
#include "VlkTextureCreationFunctions.h"
#include "VlkDevice.h"

namespace Hail
{
	VkFilter ToVkFilter(TEXTURE_FILTER_MODE mode)
	{
		switch (mode)
		{
		case Hail::TEXTURE_FILTER_MODE::NEAREST:
			return VK_FILTER_NEAREST;
		case Hail::TEXTURE_FILTER_MODE::LINEAR:
			return VK_FILTER_LINEAR;
		case Hail::TEXTURE_FILTER_MODE::CUBIC_EXT:
			return VK_FILTER_CUBIC_EXT;
		}
	}

	VkSamplerAddressMode ToVkSamplerAdressMode(TEXTURE_WRAP_MODE adressMode)
	{
		switch (adressMode)
		{
		case Hail::TEXTURE_WRAP_MODE::REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case Hail::TEXTURE_WRAP_MODE::MIRRORED_REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case Hail::TEXTURE_WRAP_MODE::CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case Hail::TEXTURE_WRAP_MODE::CLAMP_TO_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case Hail::TEXTURE_WRAP_MODE::MIRROR_CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		}
	}

	VkCompareOp ToVkCompareOperation(COMPARE_MODE mode)
	{
		switch (mode)
		{
		case Hail::COMPARE_MODE::NEVER:
			return  VK_COMPARE_OP_NEVER;
		case Hail::COMPARE_MODE::LESS:
			return  VK_COMPARE_OP_LESS;
		case Hail::COMPARE_MODE::EQUAL:
			return  VK_COMPARE_OP_EQUAL;
		case Hail::COMPARE_MODE::LESS_OR_EQUAL:
			return  VK_COMPARE_OP_LESS_OR_EQUAL;
		case Hail::COMPARE_MODE::GREATER:
			return  VK_COMPARE_OP_GREATER;
		case Hail::COMPARE_MODE::NOT_EQUAL:
			return  VK_COMPARE_OP_NOT_EQUAL;
		case Hail::COMPARE_MODE::GREATER_OR_EQUAL:
			return  VK_COMPARE_OP_GREATER_OR_EQUAL;
		case Hail::COMPARE_MODE::ALWAYS:
			return  VK_COMPARE_OP_ALWAYS;
		}
	}

	VkSamplerMipmapMode ToVkSamplerFilter(TEXTURE_SAMPLER_FILTER_MODE samplerMode)
	{
		switch (samplerMode)
		{
		case Hail::TEXTURE_SAMPLER_FILTER_MODE::POINT:
			return  VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case Hail::TEXTURE_SAMPLER_FILTER_MODE::LINEAR:
			return  VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}
	}
	


}

bool Hail::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


VkSampler Hail::CreateTextureSampler(VlkDevice& device, TextureSamplerData samplerData)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = ToVkFilter(samplerData.filter_mag);
	samplerInfo.minFilter = ToVkFilter(samplerData.filter_min);
	samplerInfo.addressModeU = ToVkSamplerAdressMode(samplerData.wrapMode_u);
	samplerInfo.addressModeV = ToVkSamplerAdressMode(samplerData.wrapMode_v);
	samplerInfo.addressModeW = ToVkSamplerAdressMode(samplerData.wrapMode_w);
	samplerInfo.anisotropyEnable = samplerData.anisotropy;
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device.GetPhysicalDevice(), &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;//TODO: Remove this and count this variable once
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = ToVkCompareOperation(samplerData.compareOp);
	samplerInfo.mipmapMode = ToVkSamplerFilter(samplerData.sampler_mode);
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	VkSampler sampler = VK_NULL_HANDLE;
	if (vkCreateSampler(device.GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create texture sampler!");
#endif
		return VK_NULL_HANDLE;
	}
	return sampler;
}

VkFormat Hail::ToVkFormat(eTextureFormat format)
{
	VkFormat returnFormat = VK_FORMAT_UNDEFINED;
	switch (format)
	{
	case Hail::eTextureFormat::UNDEFINED:
		returnFormat = VK_FORMAT_UNDEFINED;
		break;
	case Hail::eTextureFormat::R4G4_UNORM_PACK8:
		returnFormat = VK_FORMAT_R4G4_UNORM_PACK8;
		break;
	case Hail::eTextureFormat::R4G4B4A4_UNORM_PACK16:
		returnFormat = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
		break;
	case Hail::eTextureFormat::B4G4R4A4_UNORM_PACK16:
		returnFormat = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
		break;
	case Hail::eTextureFormat::R5G6B5_UNORM_PACK16:
		returnFormat = VK_FORMAT_R5G6B5_UNORM_PACK16;
		break;
	case Hail::eTextureFormat::B5G6R5_UNORM_PACK16:
		returnFormat = VK_FORMAT_B5G6R5_UNORM_PACK16;
		break;
	case Hail::eTextureFormat::R5G5B5A1_UNORM_PACK16:
		returnFormat = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
		break;
	case Hail::eTextureFormat::B5G5R5A1_UNORM_PACK16:
		returnFormat = VK_FORMAT_B5G5R5A1_UNORM_PACK16;
		break;
	case Hail::eTextureFormat::A1R5G5B5_UNORM_PACK16:
		returnFormat = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
		break;
	case Hail::eTextureFormat::R8_UNORM:
		returnFormat = VK_FORMAT_R8_UNORM;
		break;
	case Hail::eTextureFormat::R8_SNORM:
		returnFormat = VK_FORMAT_R8_SNORM;
		break;
	case Hail::eTextureFormat::R8_USCALED:
		returnFormat = VK_FORMAT_R8_USCALED;
		break;
	case Hail::eTextureFormat::R8_SSCALED:
		returnFormat = VK_FORMAT_R8_SSCALED;
		break;
	case Hail::eTextureFormat::R8_UINT:
		returnFormat = VK_FORMAT_R8_UINT;
		break;
	case Hail::eTextureFormat::R8_SINT:
		returnFormat = VK_FORMAT_R8_SINT;
		break;
	case Hail::eTextureFormat::R8_SRGB:
		returnFormat = VK_FORMAT_R8_SRGB;
		break;
	case Hail::eTextureFormat::R8G8_UNORM:
		returnFormat = VK_FORMAT_R8G8_UNORM;
		break;
	case Hail::eTextureFormat::R8G8_SNORM:
		returnFormat = VK_FORMAT_R8G8_SNORM;
		break;
	case Hail::eTextureFormat::R8G8_USCALED:
		returnFormat = VK_FORMAT_R8G8_USCALED;
		break;
	case Hail::eTextureFormat::R8G8_SSCALED:
		returnFormat = VK_FORMAT_R8G8_SSCALED;
		break;
	case Hail::eTextureFormat::R8G8_UINT:
		returnFormat = VK_FORMAT_R8G8_UINT;
		break;
	case Hail::eTextureFormat::R8G8_SINT:
		returnFormat = VK_FORMAT_R8G8_SINT;
		break;
	case Hail::eTextureFormat::R8G8_SRGB:
		returnFormat = VK_FORMAT_R8G8_SRGB;
		break;
	case Hail::eTextureFormat::R8G8B8_UNORM:
		returnFormat = VK_FORMAT_R8G8B8_UNORM;
		break;
	case Hail::eTextureFormat::R8G8B8_SNORM:
		returnFormat = VK_FORMAT_R8G8B8_SNORM;
		break;
	case Hail::eTextureFormat::R8G8B8_USCALED:
		returnFormat = VK_FORMAT_R8G8B8_USCALED;
		break;
	case Hail::eTextureFormat::R8G8B8_SSCALED:
		returnFormat = VK_FORMAT_R8G8B8_SSCALED;
		break;
	case Hail::eTextureFormat::R8G8B8_UINT:
		returnFormat = VK_FORMAT_R8G8B8_UINT;
		break;
	case Hail::eTextureFormat::R8G8B8_SINT:
		returnFormat = VK_FORMAT_R8G8B8_SINT;
		break;
	case Hail::eTextureFormat::R8G8B8_SRGB:
		returnFormat = VK_FORMAT_R8G8B8_SRGB;
		break;
	case Hail::eTextureFormat::B8G8R8_UNORM:
		returnFormat = VK_FORMAT_B8G8R8_UNORM;
		break;
	case Hail::eTextureFormat::B8G8R8_SNORM:
		returnFormat = VK_FORMAT_B8G8R8_SNORM;
		break;
	case Hail::eTextureFormat::B8G8R8_USCALED:
		returnFormat = VK_FORMAT_B8G8R8_USCALED;
		break;
	case Hail::eTextureFormat::B8G8R8_SSCALED:
		returnFormat = VK_FORMAT_B8G8R8_SSCALED;
		break;
	case Hail::eTextureFormat::B8G8R8_UINT:
		returnFormat = VK_FORMAT_B8G8R8_UINT;
		break;
	case Hail::eTextureFormat::B8G8R8_SINT:
		returnFormat = VK_FORMAT_B8G8R8_SINT;
		break;
	case Hail::eTextureFormat::B8G8R8_SRGB:
		returnFormat = VK_FORMAT_B8G8R8_SRGB;
		break;
	case Hail::eTextureFormat::R8G8B8A8_UNORM:
		returnFormat = VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case Hail::eTextureFormat::R8G8B8A8_SNORM:
		returnFormat = VK_FORMAT_R8G8B8A8_SNORM;
		break;
	case Hail::eTextureFormat::R8G8B8A8_USCALED:
		returnFormat = VK_FORMAT_R8G8B8A8_USCALED;
		break;
	case Hail::eTextureFormat::R8G8B8A8_SSCALED:
		returnFormat = VK_FORMAT_R8G8B8A8_SSCALED;
		break;
	case Hail::eTextureFormat::R8G8B8A8_UINT:
		returnFormat = VK_FORMAT_R8G8B8A8_UINT;
		break;
	case Hail::eTextureFormat::R8G8B8A8_SINT:
		returnFormat = VK_FORMAT_R8G8B8A8_SINT;
		break;
	case Hail::eTextureFormat::R8G8B8A8_SRGB:
		returnFormat = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	case Hail::eTextureFormat::B8G8R8A8_UNORM:
		returnFormat = VK_FORMAT_B8G8R8A8_UNORM;
		break;
	case Hail::eTextureFormat::B8G8R8A8_SNORM:
		returnFormat = VK_FORMAT_B8G8R8A8_SNORM;
		break;
	case Hail::eTextureFormat::B8G8R8A8_USCALED:
		returnFormat = VK_FORMAT_B8G8R8A8_USCALED;
		break;
	case Hail::eTextureFormat::B8G8R8A8_SSCALED:
		returnFormat = VK_FORMAT_B8G8R8A8_SSCALED;
		break;
	case Hail::eTextureFormat::B8G8R8A8_UINT:
		returnFormat = VK_FORMAT_B8G8R8A8_UINT;
		break;
	case Hail::eTextureFormat::B8G8R8A8_SINT:
		returnFormat = VK_FORMAT_B8G8R8A8_SINT;
		break;
	case Hail::eTextureFormat::B8G8R8A8_SRGB:
		returnFormat = VK_FORMAT_B8G8R8A8_SRGB;
		break;
	case Hail::eTextureFormat::A8B8G8R8_UNORM_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
		break;
	case Hail::eTextureFormat::A8B8G8R8_SNORM_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_SNORM_PACK32;
		break;
	case Hail::eTextureFormat::A8B8G8R8_USCALED_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_USCALED_PACK32;
		break;
	case Hail::eTextureFormat::A8B8G8R8_SSCALED_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
		break;
	case Hail::eTextureFormat::A8B8G8R8_UINT_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_UINT_PACK32;
		break;
	case Hail::eTextureFormat::A8B8G8R8_SINT_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_SINT_PACK32;
		break;
	case Hail::eTextureFormat::A8B8G8R8_SRGB_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
		break;
	case Hail::eTextureFormat::A2R10G10B10_UNORM_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		break;
	case Hail::eTextureFormat::A2R10G10B10_SNORM_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_SNORM_PACK32;
		break;
	case Hail::eTextureFormat::A2R10G10B10_USCALED_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_USCALED_PACK32;
		break;
	case Hail::eTextureFormat::A2R10G10B10_SSCALED_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
		break;
	case Hail::eTextureFormat::A2R10G10B10_UINT_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_UINT_PACK32;
		break;
	case Hail::eTextureFormat::A2R10G10B10_SINT_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_SINT_PACK32;
		break;
	case Hail::eTextureFormat::A2B10G10R10_UNORM_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		break;
	case Hail::eTextureFormat::A2B10G10R10_SNORM_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_SNORM_PACK32;
		break;
	case Hail::eTextureFormat::A2B10G10R10_USCALED_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_USCALED_PACK32;
		break;
	case Hail::eTextureFormat::A2B10G10R10_SSCALED_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
		break;
	case Hail::eTextureFormat::A2B10G10R10_UINT_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_UINT_PACK32;
		break;
	case Hail::eTextureFormat::A2B10G10R10_SINT_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_SINT_PACK32;
		break;
	case Hail::eTextureFormat::R16_UNORM:
		returnFormat = VK_FORMAT_R16_UNORM;
		break;
	case Hail::eTextureFormat::R16_SNORM:
		returnFormat = VK_FORMAT_R16_SNORM;
		break;
	case Hail::eTextureFormat::R16_USCALED:
		returnFormat = VK_FORMAT_R16_USCALED;
		break;
	case Hail::eTextureFormat::R16_SSCALED:
		returnFormat = VK_FORMAT_R16_SSCALED;
		break;
	case Hail::eTextureFormat::R16_UINT:
		returnFormat = VK_FORMAT_R16_UINT;
		break;
	case Hail::eTextureFormat::R16_SINT:
		returnFormat = VK_FORMAT_R16_SINT;
		break;
	case Hail::eTextureFormat::R16_SFLOAT:
		returnFormat = VK_FORMAT_R16_SFLOAT;
		break;
	case Hail::eTextureFormat::R16G16_UNORM:
		returnFormat = VK_FORMAT_R16G16_UNORM;
		break;
	case Hail::eTextureFormat::R16G16_SNORM:
		returnFormat = VK_FORMAT_R16G16_SNORM;
		break;
	case Hail::eTextureFormat::R16G16_USCALED:
		returnFormat = VK_FORMAT_R16G16_USCALED;
		break;
	case Hail::eTextureFormat::R16G16_SSCALED:
		returnFormat = VK_FORMAT_R16G16_SSCALED;
		break;
	case Hail::eTextureFormat::R16G16_UINT:
		returnFormat = VK_FORMAT_R16G16_UINT;
		break;
	case Hail::eTextureFormat::R16G16_SINT:
		returnFormat = VK_FORMAT_R16G16_SINT;
		break;
	case Hail::eTextureFormat::R16G16_SFLOAT:
		returnFormat = VK_FORMAT_R16G16_SFLOAT;
		break;
	case Hail::eTextureFormat::R16G16B16_UNORM:
		returnFormat = VK_FORMAT_R16G16B16_UNORM;
		break;
	case Hail::eTextureFormat::R16G16B16_SNORM:
		returnFormat = VK_FORMAT_R16G16B16_SNORM;
		break;
	case Hail::eTextureFormat::R16G16B16_USCALED:
		returnFormat = VK_FORMAT_R16G16B16_USCALED;
		break;
	case Hail::eTextureFormat::R16G16B16_SSCALED:
		returnFormat = VK_FORMAT_R16G16B16_SSCALED;
		break;
	case Hail::eTextureFormat::R16G16B16_UINT:
		returnFormat = VK_FORMAT_R16G16B16_UINT;
		break;
	case Hail::eTextureFormat::R16G16B16_SINT:
		returnFormat = VK_FORMAT_R16G16B16_SINT;
		break;
	case Hail::eTextureFormat::R16G16B16_SFLOAT:
		returnFormat = VK_FORMAT_R16G16B16_SFLOAT;
		break;
	case Hail::eTextureFormat::R16G16B16A16_UNORM:
		returnFormat = VK_FORMAT_R16G16B16A16_UNORM;
		break;
	case Hail::eTextureFormat::R16G16B16A16_SNORM:
		returnFormat = VK_FORMAT_R16G16B16A16_SNORM;
		break;
	case Hail::eTextureFormat::R16G16B16A16_USCALED:
		returnFormat = VK_FORMAT_R16G16B16A16_USCALED;
		break;
	case Hail::eTextureFormat::R16G16B16A16_SSCALED:
		returnFormat = VK_FORMAT_R16G16B16A16_SSCALED;
		break;
	case Hail::eTextureFormat::R16G16B16A16_UINT:
		returnFormat = VK_FORMAT_R16G16B16A16_UINT;
		break;
	case Hail::eTextureFormat::R16G16B16A16_SINT:
		returnFormat = VK_FORMAT_R16G16B16A16_SINT;
		break;
	case Hail::eTextureFormat::R16G16B16A16_SFLOAT:
		returnFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		break;
	case Hail::eTextureFormat::R32_UINT:
		returnFormat = VK_FORMAT_R32_UINT;
		break;
	case Hail::eTextureFormat::R32_SINT:
		returnFormat = VK_FORMAT_R32_SINT;
		break;
	case Hail::eTextureFormat::R32_SFLOAT:
		returnFormat = VK_FORMAT_R32_SFLOAT;
		break;
	case Hail::eTextureFormat::R32G32_UINT:
		returnFormat = VK_FORMAT_R32G32_UINT;
		break;
	case Hail::eTextureFormat::R32G32_SINT:
		returnFormat = VK_FORMAT_R32G32_SINT;
		break;
	case Hail::eTextureFormat::R32G32_SFLOAT:
		returnFormat = VK_FORMAT_R32G32_SFLOAT;
		break;
	case Hail::eTextureFormat::R32G32B32_UINT:
		returnFormat = VK_FORMAT_R32G32B32_UINT;
		break;
	case Hail::eTextureFormat::R32G32B32_SINT:
		returnFormat = VK_FORMAT_R32G32B32_SINT;
		break;
	case Hail::eTextureFormat::R32G32B32_SFLOAT:
		returnFormat = VK_FORMAT_R32G32B32_SFLOAT;
		break;
	case Hail::eTextureFormat::R32G32B32A32_UINT:
		returnFormat = VK_FORMAT_R32G32B32A32_UINT;
		break;
	case Hail::eTextureFormat::R32G32B32A32_SINT:
		returnFormat = VK_FORMAT_R32G32B32A32_SINT;
		break;
	case Hail::eTextureFormat::R32G32B32A32_SFLOAT:
		returnFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		break;
	case Hail::eTextureFormat::R64_UINT:
		returnFormat = VK_FORMAT_R64_UINT;
		break;
	case Hail::eTextureFormat::R64_SINT:
		returnFormat = VK_FORMAT_R64_SINT;
		break;
	case Hail::eTextureFormat::R64_SFLOAT:
		returnFormat = VK_FORMAT_R64_SFLOAT;
		break;
	case Hail::eTextureFormat::R64G64_UINT:
		returnFormat = VK_FORMAT_R64G64_UINT;
		break;
	case Hail::eTextureFormat::R64G64_SINT:
		returnFormat = VK_FORMAT_R64G64_SINT;
		break;
	case Hail::eTextureFormat::R64G64_SFLOAT:
		returnFormat = VK_FORMAT_R64G64_SFLOAT;
		break;
	case Hail::eTextureFormat::R64G64B64_UINT:
		returnFormat = VK_FORMAT_R64G64B64_UINT;
		break;
	case Hail::eTextureFormat::R64G64B64_SINT:
		returnFormat = VK_FORMAT_R64G64B64_SINT;
		break;
	case Hail::eTextureFormat::R64G64B64_SFLOAT:
		returnFormat = VK_FORMAT_R64G64B64_SFLOAT;
		break;
	case Hail::eTextureFormat::R64G64B64A64_UINT:
		returnFormat = VK_FORMAT_R64G64B64A64_UINT;
		break;
	case Hail::eTextureFormat::R64G64B64A64_SINT:
		returnFormat = VK_FORMAT_R64G64B64A64_SINT;
		break;
	case Hail::eTextureFormat::R64G64B64A64_SFLOAT:
		returnFormat = VK_FORMAT_R64G64B64A64_SFLOAT;
		break;
	case Hail::eTextureFormat::B10G11R11_UFLOAT_PACK32:
		returnFormat = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		break;
	case Hail::eTextureFormat::E5B9G9R9_UFLOAT_PACK32:
		returnFormat = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		break;
	default:
		break;
	}
	return returnFormat;
}

Hail::eTextureFormat Hail::ToInternalFromVkFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_UNDEFINED:
		return eTextureFormat::UNDEFINED;
		break;
	case VK_FORMAT_R4G4_UNORM_PACK8:
		return eTextureFormat::R4G4_UNORM_PACK8;
		break;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		return eTextureFormat::R4G4B4A4_UNORM_PACK16;
		break;
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		return eTextureFormat::B4G4R4A4_UNORM_PACK16;
		break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		return eTextureFormat::R5G6B5_UNORM_PACK16;
		break;
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		return eTextureFormat::B5G6R5_UNORM_PACK16;
		break;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		return eTextureFormat::R5G5B5A1_UNORM_PACK16;
		break;
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		return eTextureFormat::B5G5R5A1_UNORM_PACK16;
		break;
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		return eTextureFormat::A1R5G5B5_UNORM_PACK16;
		break;
	case VK_FORMAT_R8_UNORM:
		return eTextureFormat::R8_UNORM;
		break;
	case VK_FORMAT_R8_SNORM:
		return eTextureFormat::R8_SNORM;
		break;
	case VK_FORMAT_R8_USCALED:
		return eTextureFormat::R8_USCALED;
		break;
	case VK_FORMAT_R8_SSCALED:
		return eTextureFormat::R8_SSCALED;
		break;
	case VK_FORMAT_R8_UINT:
		return eTextureFormat::R8_UINT;
		break;
	case VK_FORMAT_R8_SINT:
		return eTextureFormat::R8_SINT;
		break;
	case VK_FORMAT_R8_SRGB:
		return eTextureFormat::R8_SRGB;
		break;
	case VK_FORMAT_R8G8_UNORM:
		return eTextureFormat::R8G8_UNORM;
		break;
	case VK_FORMAT_R8G8_SNORM:
		return eTextureFormat::R8G8_SNORM;
		break;
	case VK_FORMAT_R8G8_USCALED:
		return eTextureFormat::R8G8_USCALED;
		break;
	case VK_FORMAT_R8G8_SSCALED:
		return eTextureFormat::R8G8_SSCALED;
		break;
	case VK_FORMAT_R8G8_UINT:
		return eTextureFormat::R8G8_UINT;
		break;
	case VK_FORMAT_R8G8_SINT:
		return eTextureFormat::R8G8_SINT;
		break;
	case VK_FORMAT_R8G8_SRGB:
		return eTextureFormat::R8G8_SRGB;
		break;
	case VK_FORMAT_R8G8B8_UNORM:
		return eTextureFormat::R8G8B8_UNORM;
		break;
	case VK_FORMAT_R8G8B8_SNORM:
		return eTextureFormat::R8G8B8_SNORM;
		break;
	case VK_FORMAT_R8G8B8_USCALED:
		return eTextureFormat::R8G8B8_USCALED;
		break;
	case VK_FORMAT_R8G8B8_SSCALED:
		return eTextureFormat::R8G8B8_SSCALED;
		break;
	case VK_FORMAT_R8G8B8_UINT:
		return eTextureFormat::R8G8B8_UINT;
		break;
	case VK_FORMAT_R8G8B8_SINT:
		return eTextureFormat::R8G8B8_SINT;
		break;
	case VK_FORMAT_R8G8B8_SRGB:
		return eTextureFormat::R8G8B8_SRGB;
		break;
	case VK_FORMAT_B8G8R8_UNORM:
		return eTextureFormat::B8G8R8_UNORM;
		break;
	case VK_FORMAT_B8G8R8_SNORM:
		return eTextureFormat::B8G8R8_SNORM;
		break;
	case VK_FORMAT_B8G8R8_USCALED:
		return eTextureFormat::B8G8R8_USCALED;
		break;
	case VK_FORMAT_B8G8R8_SSCALED:
		return eTextureFormat::B8G8R8_SSCALED;
		break;
	case VK_FORMAT_B8G8R8_UINT:
		return eTextureFormat::B8G8R8_UINT;
		break;
	case VK_FORMAT_B8G8R8_SINT:
		return eTextureFormat::B8G8R8_SINT;
		break;
	case VK_FORMAT_B8G8R8_SRGB:
		return eTextureFormat::B8G8R8_SRGB;
		break;
	case VK_FORMAT_R8G8B8A8_UNORM:
		return eTextureFormat::R8G8B8A8_UNORM;
		break;
	case VK_FORMAT_R8G8B8A8_SNORM:
		return eTextureFormat::R8G8B8A8_SNORM;
		break;
	case VK_FORMAT_R8G8B8A8_USCALED:
		return eTextureFormat::R8G8B8A8_USCALED;
		break;
	case VK_FORMAT_R8G8B8A8_SSCALED:
		return eTextureFormat::R8G8B8A8_SSCALED;
		break;
	case VK_FORMAT_R8G8B8A8_UINT:
		return eTextureFormat::R8G8B8A8_UINT;
		break;
	case VK_FORMAT_R8G8B8A8_SINT:
		return eTextureFormat::R8G8B8A8_SINT;
		break;
	case VK_FORMAT_R8G8B8A8_SRGB:
		return eTextureFormat::R8G8B8A8_SRGB;
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
		return eTextureFormat::B8G8R8A8_UNORM;
		break;
	case VK_FORMAT_B8G8R8A8_SNORM:
		return eTextureFormat::B8G8R8A8_SNORM;
		break;
	case VK_FORMAT_B8G8R8A8_USCALED:
		return eTextureFormat::B8G8R8A8_USCALED;
		break;
	case VK_FORMAT_B8G8R8A8_SSCALED:
		return eTextureFormat::B8G8R8A8_SSCALED;
		break;
	case VK_FORMAT_B8G8R8A8_UINT:
		return eTextureFormat::B8G8R8A8_UINT;
		break;
	case VK_FORMAT_B8G8R8A8_SINT:
		return eTextureFormat::B8G8R8A8_SINT;
		break;
	case VK_FORMAT_B8G8R8A8_SRGB:
		return eTextureFormat::B8G8R8A8_SRGB;
		break;
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		return eTextureFormat::A8B8G8R8_UNORM_PACK32;
		break;
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		return eTextureFormat::A8B8G8R8_SNORM_PACK32;
		break;
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		return eTextureFormat::A8B8G8R8_USCALED_PACK32;
		break;
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		return eTextureFormat::A8B8G8R8_SSCALED_PACK32;
		break;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		return eTextureFormat::A8B8G8R8_UINT_PACK32;
		break;
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		return eTextureFormat::A8B8G8R8_SINT_PACK32;
		break;
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		return eTextureFormat::A8B8G8R8_SRGB_PACK32;
		break;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		return eTextureFormat::A2R10G10B10_UNORM_PACK32;
		break;
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		return eTextureFormat::A2R10G10B10_SNORM_PACK32;
		break;
	case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
		return eTextureFormat::A2R10G10B10_USCALED_PACK32;
		break;
	case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
		return eTextureFormat::A2R10G10B10_SSCALED_PACK32;
		break;
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		return eTextureFormat::A2R10G10B10_UINT_PACK32;
		break;
	case VK_FORMAT_A2R10G10B10_SINT_PACK32:
		return eTextureFormat::A2R10G10B10_SINT_PACK32;
		break;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		return eTextureFormat::A2B10G10R10_UNORM_PACK32;
		break;
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		return eTextureFormat::A2B10G10R10_SNORM_PACK32;
		break;
	case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
		return eTextureFormat::A2B10G10R10_USCALED_PACK32;
		break;
	case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		return eTextureFormat::A2B10G10R10_SSCALED_PACK32;
		break;
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		return eTextureFormat::A2B10G10R10_UINT_PACK32;
		break;
	case VK_FORMAT_A2B10G10R10_SINT_PACK32:
		return eTextureFormat::A2B10G10R10_SINT_PACK32;
		break;
	case VK_FORMAT_R16_UNORM:
		return eTextureFormat::R16_UNORM;
		break;
	case VK_FORMAT_R16_SNORM:
		return eTextureFormat::R16_SNORM;
		break;
	case VK_FORMAT_R16_USCALED:
		return eTextureFormat::R16_USCALED;
		break;
	case VK_FORMAT_R16_SSCALED:
		return eTextureFormat::R16_SSCALED;
		break;
	case VK_FORMAT_R16_UINT:
		return eTextureFormat::R16_UINT;
		break;
	case VK_FORMAT_R16_SINT:
		return eTextureFormat::R16_SINT;
		break;
	case VK_FORMAT_R16_SFLOAT:
		return eTextureFormat::R16_SFLOAT;
		break;
	case VK_FORMAT_R16G16_UNORM:
		return eTextureFormat::R16G16_UNORM;
		break;
	case VK_FORMAT_R16G16_SNORM:
		return eTextureFormat::R16G16_SNORM;
		break;
	case VK_FORMAT_R16G16_USCALED:
		return eTextureFormat::R16G16_USCALED;
		break;
	case VK_FORMAT_R16G16_SSCALED:
		return eTextureFormat::R16G16_SSCALED;
		break;
	case VK_FORMAT_R16G16_UINT:
		return eTextureFormat::R16G16_UINT;
		break;
	case VK_FORMAT_R16G16_SINT:
		return eTextureFormat::R16G16_SINT;
		break;
	case VK_FORMAT_R16G16_SFLOAT:
		return eTextureFormat::R16G16_SFLOAT;
		break;
	case VK_FORMAT_R16G16B16_UNORM:
		return eTextureFormat::R16G16B16_UNORM;
		break;
	case VK_FORMAT_R16G16B16_SNORM:
		return eTextureFormat::R16G16B16_SNORM;
		break;
	case VK_FORMAT_R16G16B16_USCALED:
		return eTextureFormat::R16G16B16_USCALED;
		break;
	case VK_FORMAT_R16G16B16_SSCALED:
		return eTextureFormat::R16G16B16_SSCALED;
		break;
	case VK_FORMAT_R16G16B16_UINT:
		return eTextureFormat::R16G16B16_UINT;
		break;
	case VK_FORMAT_R16G16B16_SINT:
		return eTextureFormat::R16G16B16_SINT;
		break;
	case VK_FORMAT_R16G16B16_SFLOAT:
		return eTextureFormat::R16G16B16_SFLOAT;
		break;
	case VK_FORMAT_R16G16B16A16_UNORM:
		return eTextureFormat::R16G16B16A16_UNORM;
		break;
	case VK_FORMAT_R16G16B16A16_SNORM:
		return eTextureFormat::R16G16B16A16_SNORM;
		break;
	case VK_FORMAT_R16G16B16A16_USCALED:
		return eTextureFormat::R16G16B16A16_USCALED;
		break;
	case VK_FORMAT_R16G16B16A16_SSCALED:
		return eTextureFormat::R16G16B16A16_SSCALED;
		break;
	case VK_FORMAT_R16G16B16A16_UINT:
		return eTextureFormat::R16G16B16A16_UINT;
		break;
	case VK_FORMAT_R16G16B16A16_SINT:
		return eTextureFormat::R16G16B16A16_SINT;
		break;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return eTextureFormat::R16G16B16A16_SFLOAT;
		break;
	case VK_FORMAT_R32_UINT:
		return eTextureFormat::R32_UINT;
		break;
	case VK_FORMAT_R32_SINT:
		return eTextureFormat::R32_SINT;
		break;
	case VK_FORMAT_R32_SFLOAT:
		return eTextureFormat::R32_SFLOAT;
		break;
	case VK_FORMAT_R32G32_UINT:
		return eTextureFormat::R32G32_UINT;
		break;
	case VK_FORMAT_R32G32_SINT:
		return eTextureFormat::R32G32_SINT;
		break;
	case VK_FORMAT_R32G32_SFLOAT:
		return eTextureFormat::R32G32_SFLOAT;
		break;
	case VK_FORMAT_R32G32B32_UINT:
		return eTextureFormat::R32G32B32_UINT;
		break;
	case VK_FORMAT_R32G32B32_SINT:
		return eTextureFormat::R32G32B32_SINT;
		break;
	case VK_FORMAT_R32G32B32_SFLOAT:
		return eTextureFormat::R32G32B32_SFLOAT;
		break;
	case VK_FORMAT_R32G32B32A32_UINT:
		return eTextureFormat::R32G32B32A32_UINT;
		break;
	case VK_FORMAT_R32G32B32A32_SINT:
		return eTextureFormat::R32G32B32A32_SINT;
		break;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return eTextureFormat::R32G32B32A32_SFLOAT;
		break;
	case VK_FORMAT_R64_UINT:
		return eTextureFormat::R64_UINT;
		break;
	case VK_FORMAT_R64_SINT:
		return eTextureFormat::R64_SINT;
		break;
	case VK_FORMAT_R64_SFLOAT:
		return eTextureFormat::R64_SFLOAT;
		break;
	case VK_FORMAT_R64G64_UINT:
		return eTextureFormat::R64G64_UINT;
		break;
	case VK_FORMAT_R64G64_SINT:
		return eTextureFormat::R64G64_SINT;
		break;
	case VK_FORMAT_R64G64_SFLOAT:
		return eTextureFormat::R64G64_SFLOAT;
		break;
	case VK_FORMAT_R64G64B64_UINT:
		return eTextureFormat::R64G64B64_UINT;
		break;
	case VK_FORMAT_R64G64B64_SINT:
		return eTextureFormat::R64G64B64_SINT;
		break;
	case VK_FORMAT_R64G64B64_SFLOAT:
		return eTextureFormat::R64G64B64_SFLOAT;
		break;
	case VK_FORMAT_R64G64B64A64_UINT:
		return eTextureFormat::R64G64B64A64_UINT;
		break;
	case VK_FORMAT_R64G64B64A64_SINT:
		return eTextureFormat::R64G64B64A64_SINT;
		break;
	case VK_FORMAT_R64G64B64A64_SFLOAT:
		return eTextureFormat::R64G64B64A64_SFLOAT;
		break;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		return eTextureFormat::B10G11R11_UFLOAT_PACK32;
		break;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
		return eTextureFormat::E5B9G9R9_UFLOAT_PACK32;
		break;
	case VK_FORMAT_MAX_ENUM:
		break;
	default:
		break;
	}
}

VkFormat Hail::ToVkFormat(TEXTURE_DEPTH_FORMAT format)
{
	VkFormat returnFormat = VK_FORMAT_UNDEFINED;
	switch (format)
	{
	case Hail::TEXTURE_DEPTH_FORMAT::UNDEFINED:
		returnFormat = VK_FORMAT_UNDEFINED;
		break;
	case Hail::TEXTURE_DEPTH_FORMAT::D16_UNORM:
		returnFormat = VK_FORMAT_D16_UNORM;
		break;
	case Hail::TEXTURE_DEPTH_FORMAT::X8_D24_UNORM_PACK32:
		returnFormat = VK_FORMAT_X8_D24_UNORM_PACK32;
		break;
	case Hail::TEXTURE_DEPTH_FORMAT::D32_SFLOAT:
		returnFormat = VK_FORMAT_D32_SFLOAT;
		break;
	case Hail::TEXTURE_DEPTH_FORMAT::S8_UINT:
		returnFormat = VK_FORMAT_S8_UINT;
		break;
	case Hail::TEXTURE_DEPTH_FORMAT::D16_UNORM_S8_UINT:
		returnFormat = VK_FORMAT_D16_UNORM_S8_UINT;
		break;
	case Hail::TEXTURE_DEPTH_FORMAT::D24_UNORM_S8_UINT:
		returnFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		break;
	case Hail::TEXTURE_DEPTH_FORMAT::D32_SFLOAT_S8_UINT:
		returnFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
		break;
	}
	return returnFormat;
}
