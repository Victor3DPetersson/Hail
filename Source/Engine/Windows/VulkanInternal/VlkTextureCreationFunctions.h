#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "Resources_Textures\TextureCommons.h"

namespace Hail
{

	class VlkDevice;
	bool HasStencilComponent(VkFormat format);
	VkSampler CreateTextureSampler(VlkDevice& device, TextureSamplerData samplerData);
	VkFormat ToVkFormat(eTextureFormat format);
	eTextureFormat ToInternalFromVkFormat(VkFormat format);
	VkFormat ToVkFormat(TEXTURE_DEPTH_FORMAT format);
}

