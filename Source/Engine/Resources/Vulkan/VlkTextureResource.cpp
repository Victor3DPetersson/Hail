#include "Engine_PCH.h"

#include "VlkTextureResource.h"
#include "Windows\VulkanInternal\VlkDevice.h"

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
