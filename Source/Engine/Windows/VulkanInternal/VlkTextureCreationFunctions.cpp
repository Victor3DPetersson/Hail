#include "Engine_PCH.h"
#include "VlkTextureCreationFunctions.h"
#include "VlkDevice.h"
#include "VlkSingleTimeCommand.h"

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

void Hail::CreateImage(VlkDevice& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device.GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create image!");
#endif
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device.GetDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(device, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device.GetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to allocate image memory!");
#endif
	}
	vkBindImageMemory(device.GetDevice(), image, imageMemory, 0);
}

VkImageView Hail::CreateImageView(VlkDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView = VK_NULL_HANDLE;
	if (vkCreateImageView(device.GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create texture image view!");
#endif
	}
	return imageView;
}


uint32_t Hail::FindMemoryType(VlkDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device.GetPhysicalDevice(), &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
#ifdef DEBUG
	throw std::runtime_error("failed to find suitable memory type!");
#endif
	return 0;
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

//TODO: Make image transition function to consider mip levels and the like
void Hail::TransitionImageLayout(VlkDevice& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool commandPool, VkQueue queue)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
#ifdef DEBUG
		throw std::invalid_argument("unsupported layout transition!");
#endif
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands(device, commandBuffer, queue, commandPool);

}

void Hail::CopyBufferToImage(VlkDevice& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool commandPool, VkQueue queue)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	EndSingleTimeCommands(device, commandBuffer, queue, commandPool);
}

VkFormat Hail::ToVkFormat(TEXTURE_FORMAT format)
{
	VkFormat returnFormat = VK_FORMAT_UNDEFINED;
	switch (format)
	{
	case Hail::TEXTURE_FORMAT::UNDEFINED:
		returnFormat = VK_FORMAT_UNDEFINED;
		break;
	case Hail::TEXTURE_FORMAT::R4G4_UNORM_PACK8:
		returnFormat = VK_FORMAT_R4G4_UNORM_PACK8;
		break;
	case Hail::TEXTURE_FORMAT::R4G4B4A4_UNORM_PACK16:
		returnFormat = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
		break;
	case Hail::TEXTURE_FORMAT::B4G4R4A4_UNORM_PACK16:
		returnFormat = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
		break;
	case Hail::TEXTURE_FORMAT::R5G6B5_UNORM_PACK16:
		returnFormat = VK_FORMAT_R5G6B5_UNORM_PACK16;
		break;
	case Hail::TEXTURE_FORMAT::B5G6R5_UNORM_PACK16:
		returnFormat = VK_FORMAT_B5G6R5_UNORM_PACK16;
		break;
	case Hail::TEXTURE_FORMAT::R5G5B5A1_UNORM_PACK16:
		returnFormat = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
		break;
	case Hail::TEXTURE_FORMAT::B5G5R5A1_UNORM_PACK16:
		returnFormat = VK_FORMAT_B5G5R5A1_UNORM_PACK16;
		break;
	case Hail::TEXTURE_FORMAT::A1R5G5B5_UNORM_PACK16:
		returnFormat = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
		break;
	case Hail::TEXTURE_FORMAT::R8_UNORM:
		returnFormat = VK_FORMAT_R8_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::R8_SNORM:
		returnFormat = VK_FORMAT_R8_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::R8_USCALED:
		returnFormat = VK_FORMAT_R8_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::R8_SSCALED:
		returnFormat = VK_FORMAT_R8_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::R8_UINT:
		returnFormat = VK_FORMAT_R8_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R8_SINT:
		returnFormat = VK_FORMAT_R8_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R8_SRGB:
		returnFormat = VK_FORMAT_R8_SRGB;
		break;
	case Hail::TEXTURE_FORMAT::R8G8_UNORM:
		returnFormat = VK_FORMAT_R8G8_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::R8G8_SNORM:
		returnFormat = VK_FORMAT_R8G8_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::R8G8_USCALED:
		returnFormat = VK_FORMAT_R8G8_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::R8G8_SSCALED:
		returnFormat = VK_FORMAT_R8G8_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::R8G8_UINT:
		returnFormat = VK_FORMAT_R8G8_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R8G8_SINT:
		returnFormat = VK_FORMAT_R8G8_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R8G8_SRGB:
		returnFormat = VK_FORMAT_R8G8_SRGB;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8_UNORM:
		returnFormat = VK_FORMAT_R8G8B8_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8_SNORM:
		returnFormat = VK_FORMAT_R8G8B8_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8_USCALED:
		returnFormat = VK_FORMAT_R8G8B8_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8_SSCALED:
		returnFormat = VK_FORMAT_R8G8B8_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8_UINT:
		returnFormat = VK_FORMAT_R8G8B8_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8_SINT:
		returnFormat = VK_FORMAT_R8G8B8_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8_SRGB:
		returnFormat = VK_FORMAT_R8G8B8_SRGB;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8_UNORM:
		returnFormat = VK_FORMAT_B8G8R8_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8_SNORM:
		returnFormat = VK_FORMAT_B8G8R8_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8_USCALED:
		returnFormat = VK_FORMAT_B8G8R8_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8_SSCALED:
		returnFormat = VK_FORMAT_B8G8R8_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8_UINT:
		returnFormat = VK_FORMAT_B8G8R8_UINT;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8_SINT:
		returnFormat = VK_FORMAT_B8G8R8_SINT;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8_SRGB:
		returnFormat = VK_FORMAT_B8G8R8_SRGB;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8A8_UNORM:
		returnFormat = VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8A8_SNORM:
		returnFormat = VK_FORMAT_R8G8B8A8_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8A8_USCALED:
		returnFormat = VK_FORMAT_R8G8B8A8_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8A8_SSCALED:
		returnFormat = VK_FORMAT_R8G8B8A8_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8A8_UINT:
		returnFormat = VK_FORMAT_R8G8B8A8_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8A8_SINT:
		returnFormat = VK_FORMAT_R8G8B8A8_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R8G8B8A8_SRGB:
		returnFormat = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8A8_UNORM:
		returnFormat = VK_FORMAT_B8G8R8A8_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8A8_SNORM:
		returnFormat = VK_FORMAT_B8G8R8A8_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8A8_USCALED:
		returnFormat = VK_FORMAT_B8G8R8A8_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8A8_SSCALED:
		returnFormat = VK_FORMAT_B8G8R8A8_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8A8_UINT:
		returnFormat = VK_FORMAT_B8G8R8A8_UINT;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8A8_SINT:
		returnFormat = VK_FORMAT_B8G8R8A8_SINT;
		break;
	case Hail::TEXTURE_FORMAT::B8G8R8A8_SRGB:
		returnFormat = VK_FORMAT_B8G8R8A8_SRGB;
		break;
	case Hail::TEXTURE_FORMAT::A8B8G8R8_UNORM_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A8B8G8R8_SNORM_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_SNORM_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A8B8G8R8_USCALED_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_USCALED_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A8B8G8R8_SSCALED_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A8B8G8R8_UINT_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_UINT_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A8B8G8R8_SINT_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_SINT_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A8B8G8R8_SRGB_PACK32:
		returnFormat = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2R10G10B10_UNORM_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2R10G10B10_SNORM_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_SNORM_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2R10G10B10_USCALED_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_USCALED_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2R10G10B10_SSCALED_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2R10G10B10_UINT_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_UINT_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2R10G10B10_SINT_PACK32:
		returnFormat = VK_FORMAT_A2R10G10B10_SINT_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2B10G10R10_UNORM_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2B10G10R10_SNORM_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_SNORM_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2B10G10R10_USCALED_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_USCALED_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2B10G10R10_SSCALED_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2B10G10R10_UINT_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_UINT_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::A2B10G10R10_SINT_PACK32:
		returnFormat = VK_FORMAT_A2B10G10R10_SINT_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::R16_UNORM:
		returnFormat = VK_FORMAT_R16_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::R16_SNORM:
		returnFormat = VK_FORMAT_R16_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::R16_USCALED:
		returnFormat = VK_FORMAT_R16_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::R16_SSCALED:
		returnFormat = VK_FORMAT_R16_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::R16_UINT:
		returnFormat = VK_FORMAT_R16_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R16_SINT:
		returnFormat = VK_FORMAT_R16_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R16_SFLOAT:
		returnFormat = VK_FORMAT_R16_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R16G16_UNORM:
		returnFormat = VK_FORMAT_R16G16_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::R16G16_SNORM:
		returnFormat = VK_FORMAT_R16G16_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::R16G16_USCALED:
		returnFormat = VK_FORMAT_R16G16_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::R16G16_SSCALED:
		returnFormat = VK_FORMAT_R16G16_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::R16G16_UINT:
		returnFormat = VK_FORMAT_R16G16_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R16G16_SINT:
		returnFormat = VK_FORMAT_R16G16_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R16G16_SFLOAT:
		returnFormat = VK_FORMAT_R16G16_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16_UNORM:
		returnFormat = VK_FORMAT_R16G16B16_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16_SNORM:
		returnFormat = VK_FORMAT_R16G16B16_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16_USCALED:
		returnFormat = VK_FORMAT_R16G16B16_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16_SSCALED:
		returnFormat = VK_FORMAT_R16G16B16_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16_UINT:
		returnFormat = VK_FORMAT_R16G16B16_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16_SINT:
		returnFormat = VK_FORMAT_R16G16B16_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16_SFLOAT:
		returnFormat = VK_FORMAT_R16G16B16_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16A16_UNORM:
		returnFormat = VK_FORMAT_R16G16B16A16_UNORM;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16A16_SNORM:
		returnFormat = VK_FORMAT_R16G16B16A16_SNORM;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16A16_USCALED:
		returnFormat = VK_FORMAT_R16G16B16A16_USCALED;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16A16_SSCALED:
		returnFormat = VK_FORMAT_R16G16B16A16_SSCALED;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16A16_UINT:
		returnFormat = VK_FORMAT_R16G16B16A16_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16A16_SINT:
		returnFormat = VK_FORMAT_R16G16B16A16_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R16G16B16A16_SFLOAT:
		returnFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R32_UINT:
		returnFormat = VK_FORMAT_R32_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R32_SINT:
		returnFormat = VK_FORMAT_R32_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R32_SFLOAT:
		returnFormat = VK_FORMAT_R32_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R32G32_UINT:
		returnFormat = VK_FORMAT_R32G32_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R32G32_SINT:
		returnFormat = VK_FORMAT_R32G32_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R32G32_SFLOAT:
		returnFormat = VK_FORMAT_R32G32_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R32G32B32_UINT:
		returnFormat = VK_FORMAT_R32G32B32_UINT;
			break;
	case Hail::TEXTURE_FORMAT::R32G32B32_SINT:
		returnFormat = VK_FORMAT_R32G32B32_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R32G32B32_SFLOAT:
		returnFormat = VK_FORMAT_R32G32B32_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R32G32B32A32_UINT:
		returnFormat = VK_FORMAT_R32G32B32A32_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R32G32B32A32_SINT:
		returnFormat = VK_FORMAT_R32G32B32A32_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R32G32B32A32_SFLOAT:
		returnFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R64_UINT:
		returnFormat = VK_FORMAT_R64_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R64_SINT:
		returnFormat = VK_FORMAT_R64_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R64_SFLOAT:
		returnFormat = VK_FORMAT_R64_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R64G64_UINT:
		returnFormat = VK_FORMAT_R64G64_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R64G64_SINT:
		returnFormat = VK_FORMAT_R64G64_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R64G64_SFLOAT:
		returnFormat = VK_FORMAT_R64G64_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R64G64B64_UINT:
		returnFormat = VK_FORMAT_R64G64B64_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R64G64B64_SINT:
		returnFormat = VK_FORMAT_R64G64B64_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R64G64B64_SFLOAT:
		returnFormat = VK_FORMAT_R64G64B64_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::R64G64B64A64_UINT:
		returnFormat = VK_FORMAT_R64G64B64A64_UINT;
		break;
	case Hail::TEXTURE_FORMAT::R64G64B64A64_SINT:
		returnFormat = VK_FORMAT_R64G64B64A64_SINT;
		break;
	case Hail::TEXTURE_FORMAT::R64G64B64A64_SFLOAT:
		returnFormat = VK_FORMAT_R64G64B64A64_SFLOAT;
		break;
	case Hail::TEXTURE_FORMAT::B10G11R11_UFLOAT_PACK32:
		returnFormat = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		break;
	case Hail::TEXTURE_FORMAT::E5B9G9R9_UFLOAT_PACK32:
		returnFormat = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		break;
	default:
		break;
	}
	return returnFormat;
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
