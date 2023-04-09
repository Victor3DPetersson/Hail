#pragma once

#include "InputHandler.h"
#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"

namespace Hail
{
	struct VlkMaterialDescriptor
	{
		VkDescriptorSet m_descriptor;

	};

	struct VlkTextureData
	{
		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkImageView textureImageView = VK_NULL_HANDLE;
	};
}