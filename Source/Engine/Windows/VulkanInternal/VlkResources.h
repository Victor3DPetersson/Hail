#pragma once

#include <Windows.h>
#include "VlkFrameBufferTexture.h"

namespace Hail
{
	struct VlkPassData
	{
		struct VkInternalMaterialDescriptorSet
		{
			VkDescriptorSet descriptors[MAX_FRAMESINFLIGHT * 2];
		};
		VkDescriptorSet m_passDescriptors[MAX_FRAMESINFLIGHT * 2];
		GrowingArray<VkInternalMaterialDescriptorSet> m_materialDescriptors;


		VlkFrameBufferTexture* m_frameBufferTextures;
		uint32_t numberOfFrameBufferTextures = 0;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkFramebuffer m_frameBuffer[MAX_FRAMESINFLIGHT * 2];
		
		VkDescriptorSetLayout m_materialSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_passSetLayout = VK_NULL_HANDLE;

		uint32_t m_numberOfFramesInFlight = MAX_FRAMESINFLIGHT;
	};

	struct VlkTextureData
	{
		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkImageView textureImageView = VK_NULL_HANDLE;
	};
}