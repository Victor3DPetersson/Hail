//Interface for the entire engine
#pragma once

#include "Containers\GrowingArray\GrowingArray.h"
#include "VlkTextureCreationFunctions.h"
#include "Rendering\FrameBufferTexture.h"

namespace Hail
{
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
		friend class VlkSwapChain;
	public:
		VlkFrameBufferTexture();
		VlkFrameBufferTexture(glm::uvec2 resolution, TEXTURE_FORMAT format = TEXTURE_FORMAT::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED);
		void CreateFrameBufferTextureObjects(RenderingDevice* device) override;
		//TODO: Make device be inherited from main framework and pass in a pointer to that framework here
		void ClearResources(RenderingDevice* device, bool isSwapchain = false) override;

		FrameBufferTextureData GetTextureImage(uint32_t index);
		FrameBufferTextureData GetDepthTextureImage(uint32_t index);

	private:
		void CreateTextureResources(bool isColorTexture)  override;
		void CreateTexture(VlkDevice& device);
		void CreateDepthTexture(VlkDevice& device);
		void NullMemory();
		// TODO: look in to making a seperate class for the swapchain and remove this * 2 on the frame in flight
		VkImage m_textureImage[MAX_FRAMESINFLIGHT * 2];
		VkDeviceMemory m_textureMemory[MAX_FRAMESINFLIGHT * 2];
		VkImageView m_textureView[MAX_FRAMESINFLIGHT * 2];
		VkImage m_depthTextureImage[MAX_FRAMESINFLIGHT * 2];
		VkDeviceMemory m_depthTextureMemory[MAX_FRAMESINFLIGHT * 2];
		VkImageView m_depthTextureView[MAX_FRAMESINFLIGHT * 2];
	};
}