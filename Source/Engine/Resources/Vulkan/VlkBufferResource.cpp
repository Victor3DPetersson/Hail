#include "Engine_PCH.h"

#include "VlkTextureResource.h"
#include "Windows\VulkanInternal\VlkDevice.h"
#include "VlkBufferResource.h"
#include "Windows/VulkanInternal/VlkBufferCreationFunctions.h"

using namespace Hail;

namespace
{
	void ClearTexture(VlkTextureData& textureData, VlkDevice& vlkDevice)
	{
		if (textureData.textureImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(vlkDevice.GetDevice(), textureData.textureImageView, nullptr);
		}
		if (textureData.textureImage != VK_NULL_HANDLE)
		{
			vkDestroyImage(vlkDevice.GetDevice(), textureData.textureImage, nullptr);
		}
		if (textureData.textureImageMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(vlkDevice.GetDevice(), textureData.textureImageMemory, nullptr);
		}
		textureData.textureImageView = VK_NULL_HANDLE;
		textureData.textureImage = VK_NULL_HANDLE;
		textureData.textureImageMemory = VK_NULL_HANDLE;
	}
}


void VlkTextureResource::CleanupResource(RenderingDevice* device)
{
	m_validator.MarkResourceAsDirty(0);
	ClearTexture(m_unloadingTextureData, *(VlkDevice*)device);
	ClearTexture(m_textureData, *(VlkDevice*)device);
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
		ClearTexture(m_unloadingTextureData, *(VlkDevice*)device);
	}
}

bool Hail::VlkBufferObject::Init(RenderingDevice* device, BufferProperties properties)
{
	m_properties = properties;
	VlkDevice& vlkDevice = *(VlkDevice*)device;

	VkBufferUsageFlags usageFlag;
	if (properties.type == eBufferType::uniform)
	{
		usageFlag = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	else if (properties.type == eBufferType::structured)
	{
		usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
	else
	{
		// TODO: assert
		return false;
	}

	for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		if (!CreateBuffer(vlkDevice, properties.elementByteSize * properties.numberOfElements, usageFlag, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer[i], m_bufferMemory[i]))
		{
			return false;
		}
		vkMapMemory(vlkDevice.GetDevice(), m_bufferMemory[i], properties.offset, properties.elementByteSize * properties.numberOfElements, 0, &m_bufferMapped[i]);
	}
	return true;
}

void Hail::VlkBufferObject::CleanupResource(RenderingDevice* device)
{
	VlkDevice& vkDevice = *(VlkDevice*)device;
	for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		if (m_buffer[i] != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(vkDevice.GetDevice(), m_buffer[i], nullptr);
		}
		if (m_bufferMemory[i] != VK_NULL_HANDLE)
		{
			vkFreeMemory(vkDevice.GetDevice(), m_bufferMemory[i], nullptr);
		}
		m_buffer[i] = VK_NULL_HANDLE;
		m_bufferMemory[i] = VK_NULL_HANDLE;
	}
}
