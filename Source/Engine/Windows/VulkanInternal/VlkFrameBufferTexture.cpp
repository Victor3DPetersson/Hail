#include "Engine_PCH.h"

#include "VlkFrameBufferTexture.h"
#include "VlkDevice.h"
#include "Resources\Vulkan\VlkTextureResource.h"
namespace Hail
{

	FrameBufferTextureData::FrameBufferTextureData(VkImage& image, VkDeviceMemory& memory, VkImageView& imageView) :
		image(image), memory(memory), imageView(imageView)
	{
	}

	VlkFrameBufferTexture::VlkFrameBufferTexture(glm::uvec2 resolution, eTextureFormat format, TEXTURE_DEPTH_FORMAT depthFormat) :
		FrameBufferTexture(resolution, format, depthFormat)
	{
	}

	void VlkFrameBufferTexture::CreateFrameBufferTextureObjects(RenderingDevice* pDevice)
	{
		VlkDevice* vlkDevice = (VlkDevice*)pDevice;


		uint32_t attachmentCount = 0;
		attachmentCount += m_textureFormat != eTextureFormat::UNDEFINED ? 1 : 0;
		attachmentCount += m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED ? 1 : 0;

		//TODO: assert if both are undefined
		if (m_textureFormat != eTextureFormat::UNDEFINED && m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED )
		{
			CreateTextureResources(false, vlkDevice);
			CreateTextureResources(true, vlkDevice);
		}
		else if(m_textureFormat != eTextureFormat::UNDEFINED)
		{
			CreateTextureResources(true, vlkDevice);
		}
		else if(m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED)
		{
			CreateTextureResources(false, vlkDevice);
		}
	}

	void VlkFrameBufferTexture::CreateTextureResources(bool bIsColorTexture, RenderingDevice* pDevice)
	{
		TextureProperties props{};
		props.width = m_resolution.x;
		props.height = m_resolution.y;
		props.format = m_textureFormat;
		VlkDevice* pVlkDevice = (VlkDevice*)pDevice;

		for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			VlkTextureResource* vlkTextureResource = new VlkTextureResource();
			vlkTextureResource->textureName = m_bufferName;
			if (bIsColorTexture)
			{
				props.depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED;
				props.textureUsage = eTextureUsage::FramebufferColor;
			}
			else
			{
				props.depthFormat = m_depthFormat;
				props.textureUsage = eTextureUsage::FramebufferDepthOnly;
			}
			vlkTextureResource->m_properties = props;
			vlkTextureResource->m_index = MAX_UINT - 2;
			H_ASSERT(vlkTextureResource->Init(pDevice), "Failed creating frame buffer texture");

			VlkTextureView* pVlkTextureView = new VlkTextureView();
			TextureViewProperties viewProps{};
			viewProps.viewUsage = props.textureUsage;
			viewProps.pTextureToView = vlkTextureResource;
			H_ASSERT(pVlkTextureView->InitView(pDevice, viewProps));

			if (bIsColorTexture)
			{
				m_pTextureResource[i] = vlkTextureResource;
				m_pTextureViews[i] = pVlkTextureView;
			}
			else
			{
				m_pDepthTextureResource[i] = vlkTextureResource;
				m_pDepthTextureViews[i] = pVlkTextureView;
			}
		}

	}
}
