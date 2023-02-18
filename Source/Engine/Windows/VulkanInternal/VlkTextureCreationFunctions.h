//Interface for the entire engine
#pragma once

#include "Renderer.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"

namespace Hail
{
	class VlkDevice;
	VkImageView  CreateImageView(VlkDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateImage(VlkDevice& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	uint32_t FindMemoryType(VlkDevice& device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	bool HasStencilComponent(VkFormat format);

}

