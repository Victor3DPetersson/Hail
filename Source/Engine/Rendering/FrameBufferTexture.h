#pragma once

#include "glm\vec2.hpp"
#include "Resources\Resource.h"
#include "String.hpp"

namespace Hail
{
	class FrameBufferTexture
	{
	public:
		FrameBufferTexture() = default;
		FrameBufferTexture(glm::uvec2 resolution, TEXTURE_FORMAT format = TEXTURE_FORMAT::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED);

		TEXTURE_FORMAT GetTextureFormat() { return m_textureFormat; }
		TEXTURE_DEPTH_FORMAT GetDepthFormat() { return m_depthFormat; }
		glm::uvec2 GetResolution() { return m_resolution; }

		void SetName(String64 bufferName) { m_bufferName = bufferName; }
		void SetTextureFormat(TEXTURE_FORMAT format) { m_textureFormat = format; }
		void SetDepthFormat(TEXTURE_DEPTH_FORMAT format) { m_depthFormat = format; }
		void SetResolution(glm::uvec2 resolution) { m_resolution = resolution; }
		void SetBindingPoint(uint32_t bindingPoint) { m_bindingPoint = bindingPoint; }
		void SetClearColor(glm::vec3 clearColor) { m_clearColor = clearColor; }

		bool HasDepthAttachment() { return (m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED); }

	protected:
		String64 m_bufferName;
		bool m_isBoundAsTarget = false;
		uint32_t m_bindingPoint = UINT_MAX;
		TEXTURE_FORMAT m_textureFormat = TEXTURE_FORMAT::UNDEFINED;
		TEXTURE_DEPTH_FORMAT m_depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED;
		glm::uvec2 m_resolution;
		glm::vec3 m_clearColor = Color_BLACK;
	};
}

