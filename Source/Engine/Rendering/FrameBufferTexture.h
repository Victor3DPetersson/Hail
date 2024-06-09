#pragma once

#include "ResourceCommon.h"
#include "Resources_Textures\TextureCommons.h"
#include "Containers\StaticArray\StaticArray.h"
#include "String.hpp"

namespace Hail
{
	class RenderingDevice;
	class TextureResource;

	class FrameBufferTexture
	{
	public:
		FrameBufferTexture() = default;
		FrameBufferTexture(glm::uvec2 resolution, TEXTURE_FORMAT format = TEXTURE_FORMAT::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED);
		
		virtual void CreateFrameBufferTextureObjects(RenderingDevice* device) = 0;
		virtual void ClearResources(RenderingDevice* device, bool isSwapchain = false) = 0;

		TEXTURE_FORMAT GetTextureFormat() { return m_textureFormat; }
		TEXTURE_DEPTH_FORMAT GetDepthFormat() { return m_depthFormat; }
		glm::uvec2 GetResolution() { return m_resolution; }
		glm::vec3 GetClearColor() { return m_clearColor; }

		void SetName(String64 bufferName) { m_bufferName = bufferName; }
		void SetTextureFormat(TEXTURE_FORMAT format) { m_textureFormat = format; }
		void SetDepthFormat(TEXTURE_DEPTH_FORMAT format) { m_depthFormat = format; }
		void SetResolution(glm::uvec2 resolution) { m_resolution = resolution; }
		void SetBindingPoint(uint32 bindingPoint) { m_bindingPoint = bindingPoint; }
		void SetClearColor(glm::vec3 clearColor) { m_clearColor = clearColor; }

		bool HasDepthAttachment() { return (m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED); }

		TextureResource* GetColorTexture(uint32 frameInFlight) { return m_pTextureResource[frameInFlight]; }
		TextureResource* GetDepthTexture(uint32 frameInFlight) { return m_pDepthTextureResource[frameInFlight]; }

	protected:

		virtual void CreateTextureResources(bool bIsColorTexture) = 0;

		String64 m_bufferName;
		bool m_isBoundAsTarget = false;
		uint32 m_bindingPoint = UINT_MAX;
		TEXTURE_FORMAT m_textureFormat = TEXTURE_FORMAT::UNDEFINED;
		TEXTURE_DEPTH_FORMAT m_depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED;
		glm::uvec2 m_resolution;
		glm::vec3 m_clearColor = Color_BLACK;

		StaticArray<TextureResource*, MAX_FRAMESINFLIGHT> m_pTextureResource = nullptr;
		StaticArray<TextureResource*, MAX_FRAMESINFLIGHT> m_pDepthTextureResource = nullptr;
	};
}

