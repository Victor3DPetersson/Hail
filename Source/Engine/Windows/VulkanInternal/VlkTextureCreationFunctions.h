#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "Resources_Textures\TextureCommons.h"

namespace Hail
{

	class VlkDevice;
	VkImageView  CreateImageView(VlkDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateImage(VlkDevice& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	uint32_t FindMemoryType(VlkDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	bool HasStencilComponent(VkFormat format);
	VkSampler CreateTextureSampler(VlkDevice& device, TextureSamplerData samplerData);
	void TransitionImageLayout(VlkDevice& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool commandPool, VkQueue queue);
	void CopyBufferToImage(VlkDevice& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool commandPool, VkQueue queue);

	VkFormat ToVkFormat(eTextureFormat format);
	eTextureFormat ToInternalFromVkFormat(VkFormat format);
	VkFormat ToVkFormat(TEXTURE_DEPTH_FORMAT format);
}

