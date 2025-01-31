#include "Engine_PCH.h"

#include "VlkTextureResource.h"
#include "Windows\VulkanInternal\VlkDevice.h"

using namespace Hail;

namespace
{

	void ClearTextureData(VlkTextureResource::VlkTextureInternalData& textureData, VlkDevice& vlkDevice)
	{
		if (textureData.textureImage != VK_NULL_HANDLE)
		{
			vmaDestroyImage(vlkDevice.GetMemoryAllocator(), textureData.textureImage, textureData.allocation);
		}
		textureData.textureImage = VK_NULL_HANDLE;
		textureData.allocation = VK_NULL_HANDLE;
	}
}


void VlkTextureResource::CleanupResource(RenderingDevice* device)
{
	m_validator.MarkResourceAsDirty(0);
	ClearTextureData(m_unloadingTextureData, *(VlkDevice*)device);
	ClearTextureData(m_textureData, *(VlkDevice*)device);
}

void Hail::VlkTextureResource::CleanupResourceForReload(RenderingDevice* device, uint32 frameInFligth)
{
	if (!m_validator.GetIsResourceDirty())
	{
		m_validator.MarkResourceAsDirty(frameInFligth);
		m_unloadingTextureData = m_textureData;
	}
	else
	{
		ClearTextureData(m_unloadingTextureData, *(VlkDevice*)device);
	}
}

bool Hail::VlkTextureResource::InternalInit(RenderingDevice* pDevice)
{
	VkImageCreateInfo imgCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imgCreateInfo.extent.width = m_properties.width;
	imgCreateInfo.extent.height = m_properties.height;
	imgCreateInfo.extent.depth = 1;
	imgCreateInfo.mipLevels = 1;
	imgCreateInfo.arrayLayers = 1;
	imgCreateInfo.format = m_properties.depthFormat == TEXTURE_DEPTH_FORMAT::UNDEFINED ? ToVkFormat(m_properties.format) : ToVkFormat(m_properties.depthFormat);
	imgCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageUsageFlags usage;

	if (m_properties.textureUsage != eTextureUsage::Texture)
	{
		// TODO figure out if I need VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT on all types or if I can flag this nicely
		if (m_properties.textureUsage == eTextureUsage::FramebufferColor)
		{
			usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		}
		else if (m_properties.textureUsage == eTextureUsage::FramebufferDepthOnly)
		{
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		else if (m_properties.textureUsage == eTextureUsage::FramebufferDepthColor)
		{
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		}
	}
	else
	{
		usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	}

	imgCreateInfo.usage = usage;
	imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocCreateInfo.priority = 1.0f;
	VmaAllocator allocator = ((VlkDevice*)pDevice)->GetMemoryAllocator();
 	VkResult result = vmaCreateImage(allocator, &imgCreateInfo, &allocCreateInfo, &m_textureData.textureImage, &m_textureData.allocation, nullptr);

	return result == VK_SUCCESS;
}

void Hail::VlkTextureView::CleanupResource(RenderingDevice* pDevice)
{
	VlkDevice& vlkDevice = *(VlkDevice*)pDevice;
	if (m_textureImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(vlkDevice.GetDevice(), m_textureImageView, nullptr);
	}
	m_textureImageView = VK_NULL_HANDLE;
}

bool Hail::VlkTextureView::InitView(RenderingDevice* pDevice, TextureViewProperties properties)
{
	const TextureProperties& props = properties.pTextureToView->m_properties;
	VlkDevice& vlkDevice = *(VlkDevice*)pDevice;
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = ((VlkTextureResource*)properties.pTextureToView)->GetVlkTextureData().textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = props.depthFormat == TEXTURE_DEPTH_FORMAT::UNDEFINED ? ToVkFormat(props.format) : ToVkFormat(props.depthFormat);

	VkImageAspectFlags aspectMask{};
	if (properties.viewUsage == eTextureUsage::FramebufferColor || properties.viewUsage == eTextureUsage::Texture)
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	else if (properties.viewUsage == eTextureUsage::FramebufferDepthOnly)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (properties.viewUsage == eTextureUsage::FramebufferDepthColor)
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	if (HasStencilComponent(ToVkFormat(props.depthFormat)))
	{
		aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	viewInfo.subresourceRange.aspectMask = aspectMask;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	H_ASSERT(vkCreateImageView(vlkDevice.GetDevice(), &viewInfo, nullptr, &m_textureImageView) == VK_SUCCESS);

	m_textureIndex = properties.pTextureToView->m_index;

	return true;
}
