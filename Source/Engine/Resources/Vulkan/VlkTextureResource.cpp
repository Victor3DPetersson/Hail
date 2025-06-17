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
	VkImageUsageFlags usage{};

	if (m_properties.textureUsage == eTextureUsage::Texture)
	{
		if (m_properties.accessQualifier == eShaderAccessQualifier::ReadOnly)
		{
			usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		else if (m_properties.accessQualifier == eShaderAccessQualifier::ReadWrite)
		{
			usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		}
		else
		{
			usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		}
	}
	else // Framebuffer textures
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

	imgCreateInfo.usage = usage;
	imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocCreateInfo.priority = 1.0f;
	VmaAllocator allocator = ((VlkDevice*)pDevice)->GetMemoryAllocator();
 	VkResult result = vmaCreateImage(allocator, &imgCreateInfo, &allocCreateInfo, &m_textureData.textureImage, &m_textureData.allocation, nullptr);
	
	m_textureData.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	m_textureData.currentStageUsage = 0u;

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
	H_ASSERT(properties.pTextureToView);

	bool bValidViewForTexture = properties.accessQualifier == eShaderAccessQualifier::ReadWrite;

	if (!bValidViewForTexture)
	{
		if (properties.pTextureToView->m_accessQualifier == eShaderAccessQualifier::ReadOnly)
		{
			bValidViewForTexture = properties.accessQualifier == eShaderAccessQualifier::ReadOnly;
		}
		else if (properties.pTextureToView->m_accessQualifier == eShaderAccessQualifier::WriteOnly)
		{
			bValidViewForTexture = properties.accessQualifier == eShaderAccessQualifier::WriteOnly;
		}
		else
		{
			bValidViewForTexture = true;
		}
	}

	if (!bValidViewForTexture)
	{
		H_ERROR("Failed to init texture view");
		return false;
	}

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
	m_props = properties;

	return true;
}

void Hail::VlkSamplerObject::Init(RenderingDevice* pDevice, SamplerProperties props)
{
	VlkDevice& vlkDevice = *(VlkDevice*)pDevice;
	m_props = props;

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = ToVkFilter(m_props.filter_mag);
	samplerInfo.minFilter = ToVkFilter(m_props.filter_min);
	samplerInfo.addressModeU = ToVkSamplerAdressMode(m_props.wrapMode_u);
	samplerInfo.addressModeV = ToVkSamplerAdressMode(m_props.wrapMode_v);
	samplerInfo.addressModeW = ToVkSamplerAdressMode(m_props.wrapMode_w);
	samplerInfo.anisotropyEnable = m_props.anisotropy;
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(vlkDevice.GetPhysicalDevice(), &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;//TODO: Remove this and count this variable once
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = ToVkCompareOperation(m_props.compareOp);
	samplerInfo.mipmapMode = ToVkSamplerFilter(m_props.sampler_mode);
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	vkCreateSampler(vlkDevice.GetDevice(), &samplerInfo, nullptr, &m_sampler);
	H_ASSERT(m_sampler != VK_NULL_HANDLE, "Failed to create Sampler");
}

void Hail::VlkSamplerObject::CleanupResource(RenderingDevice* pDevice)
{
	VlkDevice* device = (VlkDevice*)pDevice;
	vkDestroySampler(device->GetDevice(), m_sampler, nullptr);
}
