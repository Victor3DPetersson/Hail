#include "Engine_PCH.h"

#include "VlkFrameBufferTexture.h"
#include "VlkTextureCreationFunctions.h"
#include "VlkDevice.h"

namespace Hail
{

	FrameBufferTextureData::FrameBufferTextureData(VkImage& image, VkDeviceMemory& memory, VkImageView& imageView) :
		image(image), memory(memory), imageView(imageView)
	{
	}

	VlkFrameBufferTexture::VlkFrameBufferTexture(glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) :
		FrameBufferTexture(resolution, format, depthFormat)
	{
	}

	void VlkFrameBufferTexture::CreateFrameBufferTextureObjects(VlkDevice& device)
	{
		uint32_t attachmentCount = 0;
		attachmentCount += m_textureFormat != TEXTURE_FORMAT::UNDEFINED ? 1 : 0;
		attachmentCount += m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED ? 1 : 0;

		VkImageView attachments[2];

		if (m_textureFormat != TEXTURE_FORMAT::UNDEFINED && m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED )
		{
			CreateTexture(device);
			CreateDepthTexture(device);
			attachments[0] = m_textureView;
			attachments[1] = m_depthTextureView;
		}
		else if(m_textureFormat != TEXTURE_FORMAT::UNDEFINED)
		{
			CreateTexture(device);
			attachments[0] = m_textureView;
		}
		else if(m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED)
		{
			CreateDepthTexture(device);
			attachments[0] = m_depthTextureView;
		}
	}



	FrameBufferTextureData Hail::VlkFrameBufferTexture::GetTextureImage()
	{
		return FrameBufferTextureData(m_textureImage, m_textureMemory, m_textureView);
	}

	FrameBufferTextureData Hail::VlkFrameBufferTexture::GetDepthTextureImage()
	{
		return FrameBufferTextureData(m_depthTextureImage, m_depthTextureMemory, m_depthTextureView);
	}

	void VlkFrameBufferTexture::CreateTexture(VlkDevice& device)
	{
		VkFormat textureFormat = ToVkFormat(m_textureFormat);
		CreateImage(device, m_resolution.x, m_resolution.y,
			textureFormat, VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_textureImage, m_textureMemory);
		m_textureView = CreateImageView(device, m_textureImage, textureFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void VlkFrameBufferTexture::CreateDepthTexture(VlkDevice& device)
	{
		VkFormat depthFormat = ToVkFormat(m_depthFormat);
		CreateImage(device, m_resolution.x, m_resolution.y,
			depthFormat, VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_depthTextureImage, m_depthTextureMemory);
		if (HasStencilComponent(depthFormat))
		{
			m_depthTextureView = CreateImageView(device, m_depthTextureImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		}
		else
		{
			m_depthTextureView = CreateImageView(device, m_depthTextureImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		}
	}

}
