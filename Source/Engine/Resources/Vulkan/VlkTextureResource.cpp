#include "Engine_PCH.h"

#include "VlkTextureResource.h"
#include "Windows\VulkanInternal\VlkDevice.h"

using namespace Hail;

void VlkTextureResource::CleanupResource(RenderingDevice* device)
{
	VlkDevice& vlkDevice = *(VlkDevice*)device;
	if (m_textureData.textureImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(vlkDevice.GetDevice(), m_textureData.textureImageView, nullptr);
	}
	if (m_textureData.textureImage != VK_NULL_HANDLE)
	{
		vkDestroyImage(vlkDevice.GetDevice(), m_textureData.textureImage, nullptr);
	}
	if (m_textureData.textureImageMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(vlkDevice.GetDevice(), m_textureData.textureImageMemory, nullptr);
	}
	m_textureData.textureImageView = VK_NULL_HANDLE;
	m_textureData.textureImage = VK_NULL_HANDLE;
	m_textureData.textureImageMemory = VK_NULL_HANDLE;
}


