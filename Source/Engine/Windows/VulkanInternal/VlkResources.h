#pragma once

#include <Windows.h>
#include "VlkFrameBufferTexture.h"
#include "Types.h"

namespace Hail
{
	class VlkDevice;

	struct VlkTextureData
	{
		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkImageView textureImageView = VK_NULL_HANDLE;
	};
}