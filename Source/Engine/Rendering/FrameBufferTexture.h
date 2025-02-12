#pragma once

#include "ResourceCommon.h"
#include "Resources_Textures\TextureCommons.h"
#include "Containers\StaticArray\StaticArray.h"
#include "String.hpp"

namespace Hail
{
	class RenderContext;
	class RenderingDevice;
	class TextureResource;
	class TextureView;

	class FrameBufferTexture
	{
	public:
		explicit FrameBufferTexture(glm::uvec2 resolution, eTextureFormat format = eTextureFormat::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED);

		virtual void CreateFrameBufferTextureObjects(RenderingDevice* device) = 0;
		virtual void ClearResources(RenderingDevice* device, bool isSwapchain = false);

		eTextureFormat GetTextureFormat() { return m_textureFormat; }
		TEXTURE_DEPTH_FORMAT GetDepthFormat() { return m_depthFormat; }
		glm::uvec2 GetResolution() { return m_resolution; }
		glm::vec3 GetClearColor() { return m_clearColor; }

		void SetName(String64 bufferName) { m_bufferName = bufferName; }
		void SetTextureFormat(eTextureFormat format) { m_textureFormat = format; }
		void SetDepthFormat(TEXTURE_DEPTH_FORMAT format) { m_depthFormat = format; }
		void SetResolution(glm::uvec2 resolution) { m_resolution = resolution; }
		void SetClearColor(glm::vec3 clearColor) { m_clearColor = clearColor; }

		bool HasDepthAttachment() { return (m_depthFormat != TEXTURE_DEPTH_FORMAT::UNDEFINED); }

		TextureResource* GetColorTexture(uint32 frameInFlight) { return m_pTextureResource[frameInFlight]; }
		TextureView* GetColorTextureView(uint32 frameInFlight) { return m_pTextureViews[frameInFlight]; }
		TextureResource* GetDepthTexture(uint32 frameInFlight) { return m_pDepthTextureResource[frameInFlight]; }
		TextureView* GetDepthTextureView(uint32 frameInFlight) { return m_pDepthTextureViews[frameInFlight]; }

	protected:
		friend class RenderContext;
		virtual void CreateTextureResources(bool bIsColorTexture, RenderingDevice* device) = 0;

		String64 m_bufferName;
		eTextureFormat m_textureFormat = eTextureFormat::UNDEFINED;
		TEXTURE_DEPTH_FORMAT m_depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED;
		glm::uvec2 m_resolution;
		glm::vec3 m_clearColor = Color_BLACK;


		//struct FrameData
		//{
		//	TextureResource* pColorTexture;
		//	TextureResource* pDepthTexture;

		//	TextureView* pColorView;
		//	TextureView* pDepthView;

		//	eFrameBufferLayoutState colorState;
		//	eFrameBufferLayoutState depthState;
		//};
		//// TODO implementera data strukturen ovanför och ta bort nedanför listor

		//StaticArray<FrameData, MAX_FRAMESINFLIGHT> m_frameData;

		StaticArray<TextureResource*, MAX_FRAMESINFLIGHT> m_pTextureResource;
		StaticArray<TextureResource*, MAX_FRAMESINFLIGHT> m_pDepthTextureResource;

		StaticArray<TextureView*, MAX_FRAMESINFLIGHT> m_pTextureViews;
		StaticArray<TextureView*, MAX_FRAMESINFLIGHT> m_pDepthTextureViews;

		StaticArray<eFrameBufferLayoutState, MAX_FRAMESINFLIGHT> m_currentColorLayoutState;
		StaticArray<eFrameBufferLayoutState, MAX_FRAMESINFLIGHT> m_currentDepthLayoutState;
	};
}

