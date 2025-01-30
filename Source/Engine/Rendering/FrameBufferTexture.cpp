#include "Engine_PCH.h"
#include "FrameBufferTexture.h"
#include "Resources\TextureResource.h"

namespace Hail
{
	FrameBufferTexture::FrameBufferTexture(glm::uvec2 resolution, eTextureFormat format, TEXTURE_DEPTH_FORMAT depthFormat) :
		m_resolution(resolution), m_textureFormat(format), m_depthFormat(depthFormat)
	{
		m_pTextureResource.Fill(nullptr);
		m_pDepthTextureResource.Fill(nullptr);
		m_pTextureViews.Fill(nullptr);
		m_pDepthTextureViews.Fill(nullptr);
	}

	void FrameBufferTexture::ClearResources(RenderingDevice* device, bool isSwapchain)
	{
		for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			if (m_pTextureResource[i])
			{
				m_pTextureResource[i]->CleanupResource(device);
				SAFEDELETE(m_pTextureResource[i]);
				m_pTextureViews[i]->CleanupResource(device);
				SAFEDELETE(m_pTextureViews[i]);
			}
			if (m_pDepthTextureResource[i])
			{
				m_pDepthTextureResource[i]->CleanupResource(device);
				SAFEDELETE(m_pDepthTextureResource[i]);
				m_pDepthTextureViews[i]->CleanupResource(device);
				SAFEDELETE(m_pDepthTextureViews[i]);
			}
		}
	}
}