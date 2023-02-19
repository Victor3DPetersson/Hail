//Interface for the entire engine
#pragma once

#include "Renderer.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"

namespace Hail
{
	class VlkDevice;
	VkCommandBuffer BeginSingleTimeCommands(VlkDevice& device, VkCommandPool commandPool);
	void EndSingleTimeCommands(VlkDevice& device, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool);

}

