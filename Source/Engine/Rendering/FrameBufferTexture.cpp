#include "Engine_PCH.h"
#include "FrameBufferTexture.h"

namespace Hail
{
	FrameBufferTexture::FrameBufferTexture(glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) :
		m_resolution(resolution), m_textureFormat(format), m_depthFormat(depthFormat)
	{
	}
}