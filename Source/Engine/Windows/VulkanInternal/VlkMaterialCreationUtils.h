#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"

namespace Hail
{
	class Material;
	VkPipelineColorBlendAttachmentState CreateColorBlendAttachment(const Material& material);
}
