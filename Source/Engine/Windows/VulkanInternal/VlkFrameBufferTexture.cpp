#include "Engine_PCH.h"

#include "VlkFrameBufferTexture.h"
#include "VlkDevice.h"

namespace Hail
{

	FrameBufferTextureData::FrameBufferTextureData(VkImage& image, VkDeviceMemory& memory, VkImageView& imageView) :
		image(image), memory(memory), imageView(imageView)
	{
	}

	VlkFrameBufferTexture::VlkFrameBufferTexture() : FrameBufferTexture()
	{
		NullMemory();
	}

	VlkFrameBufferTexture::VlkFrameBufferTexture(glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) :
		FrameBufferTexture(resolution, format, depthFormat)
	{
		NullMemory();
	}

	void VlkFrameBufferTexture::CreateFrameBufferTextureObjects(RenderingDevice* device)
	{
		VlkDevice* vlkDevice = (VlkDevice*)device;


		uint32_t attachmentCount = 0;
		attachmentCount += m_textureFormat != TEXTURE_FORMAT::UNDEFINED ? 1 : 0;
		attachmentCount += m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED ? 1 : 0;

		if (m_textureFormat != TEXTURE_FORMAT::UNDEFINED && m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED )
		{
			CreateTexture(*vlkDevice);
			CreateDepthTexture(*vlkDevice);
		}
		else if(m_textureFormat != TEXTURE_FORMAT::UNDEFINED)
		{
			CreateTexture(*vlkDevice);
		}
		else if(m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED)
		{
			CreateDepthTexture(*vlkDevice);
		}
	}

	void VlkFrameBufferTexture::ClearResources(RenderingDevice* device, bool isSwapChain)
	{
		VlkDevice* vlkDevice = (VlkDevice*)device;
		for (size_t i = 0; i < MAX_FRAMESINFLIGHT * 2; i++)
		{
			if(m_textureImage[i] && !isSwapChain)
			{
				vkDestroyImage(vlkDevice->GetDevice(), m_textureImage[i], nullptr);
			}
			m_textureImage[i] = nullptr;
			if (m_textureView[i])
			{
				vkDestroyImageView(vlkDevice->GetDevice(), m_textureView[i], nullptr);
				m_textureView[i] = nullptr;
			}
			if (m_textureMemory[i])
			{
				vkFreeMemory(vlkDevice->GetDevice(), m_textureMemory[i], nullptr);
				m_textureMemory[i] = nullptr;
			}
			if (m_depthTextureImage[i])
			{
				
				vkDestroyImageView(vlkDevice->GetDevice(), m_depthTextureView[i], nullptr);
				vkFreeMemory(vlkDevice->GetDevice(), m_depthTextureMemory[i], nullptr);
				vkDestroyImage(vlkDevice->GetDevice(), m_depthTextureImage[i], nullptr);
			}
		}
	}

	FrameBufferTextureData Hail::VlkFrameBufferTexture::GetTextureImage(uint32_t index)
	{
		return FrameBufferTextureData(m_textureImage[index], m_textureMemory[index], m_textureView[index]);
	}

	FrameBufferTextureData Hail::VlkFrameBufferTexture::GetDepthTextureImage(uint32_t index)
	{
		return FrameBufferTextureData(m_depthTextureImage[index], m_depthTextureMemory[index], m_depthTextureView[index]);
	}

	void VlkFrameBufferTexture::CreateTexture(VlkDevice& device)
	{
		VkFormat textureFormat = ToVkFormat(m_textureFormat);
		for (uint32_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			CreateImage(device, m_resolution.x, m_resolution.y,
				textureFormat, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_textureImage[i], m_textureMemory[i]);
			m_textureView[i] = CreateImageView(device, m_textureImage[i], textureFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}

	}

	void VlkFrameBufferTexture::CreateDepthTexture(VlkDevice& device)
	{
		VkFormat depthFormat = ToVkFormat(m_depthFormat);
		for (uint32_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			CreateImage(device, m_resolution.x, m_resolution.y,
				depthFormat, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_depthTextureImage[i], m_depthTextureMemory[i]);
			if (HasStencilComponent(depthFormat))
			{
				m_depthTextureView[i] = CreateImageView(device, m_depthTextureImage[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
			}
			else
			{
				m_depthTextureView[i] = CreateImageView(device, m_depthTextureImage[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
			}
		}

	}

	void VlkFrameBufferTexture::NullMemory()
	{
		m_textureImage[0] = VK_NULL_HANDLE;
		m_textureMemory[0] = VK_NULL_HANDLE;
		m_textureView[0] = VK_NULL_HANDLE;
		m_depthTextureImage[0] = VK_NULL_HANDLE;
		m_depthTextureMemory[0] = VK_NULL_HANDLE;
		m_depthTextureView[0] = VK_NULL_HANDLE;

		m_textureImage[1] = VK_NULL_HANDLE;
		m_textureMemory[1] = VK_NULL_HANDLE;
		m_textureView[1] = VK_NULL_HANDLE;
		m_depthTextureImage[1] = VK_NULL_HANDLE;
		m_depthTextureMemory[1] = VK_NULL_HANDLE;
		m_depthTextureView[1] = VK_NULL_HANDLE;

		m_textureImage[2] = VK_NULL_HANDLE;
		m_textureMemory[2] = VK_NULL_HANDLE;
		m_textureView[2] = VK_NULL_HANDLE;
		m_depthTextureImage[2] = VK_NULL_HANDLE;
		m_depthTextureMemory[2] = VK_NULL_HANDLE;
		m_depthTextureView[2] = VK_NULL_HANDLE;

		m_textureImage[3] = VK_NULL_HANDLE;
		m_textureMemory[3] = VK_NULL_HANDLE;
		m_textureView[3] = VK_NULL_HANDLE;
		m_depthTextureImage[3] = VK_NULL_HANDLE;
		m_depthTextureMemory[3] = VK_NULL_HANDLE;
		m_depthTextureView[3] = VK_NULL_HANDLE;
	}

}
