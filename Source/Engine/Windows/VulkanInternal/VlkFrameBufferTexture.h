//Interface for the entire engine
#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "VlkTextureCreationFunctions.h"
#include "Containers\GrowingArray\GrowingArray.h"

#include "Rendering\FrameBufferTexture.h"

namespace Hail
{
	class VlkDevice;

	struct FrameBufferTextureData
	{
		FrameBufferTextureData() = delete;
		FrameBufferTextureData(VkImage& image, VkDeviceMemory& memory, VkImageView& imageView);
		VkImage& image;
		VkDeviceMemory& memory;
		VkImageView& imageView;
	};

	class VlkFrameBufferTexture : public FrameBufferTexture
	{
	public:
		VlkFrameBufferTexture(glm::uvec2 resolution, TEXTURE_FORMAT format = TEXTURE_FORMAT::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED);
		void CreateFrameBufferTextureObjects(VlkDevice& device);
		//TODO: Make device be inherited from main framework and pass in a pointer to that framework here
		void CleanupResources();

		FrameBufferTextureData GetTextureImage(uint32_t index);
		FrameBufferTextureData GetDepthTextureImage(uint32_t index);

	private:

		void CreateTexture(VlkDevice& device);
		void CreateDepthTexture(VlkDevice& device);

		VkImage m_textureImage[MAX_FRAMESINFLIGHT];
		VkDeviceMemory m_textureMemory[MAX_FRAMESINFLIGHT];
		VkImageView m_textureView[MAX_FRAMESINFLIGHT];
		VkImage m_depthTextureImage [MAX_FRAMESINFLIGHT];
		VkDeviceMemory m_depthTextureMemory [MAX_FRAMESINFLIGHT];
		VkImageView m_depthTextureView [MAX_FRAMESINFLIGHT];
	};
}