#pragma once

#include <Windows.h>
#include "VlkFrameBufferTexture.h"
#include "Types.h"

namespace Hail
{
	class VlkDevice;
	struct VlkPassData
	{
		struct VkInternalMaterialDescriptorSet
		{
			VkDescriptorSet descriptors[MAX_FRAMESINFLIGHT];
		};
		VkDescriptorSet m_passDescriptors[MAX_FRAMESINFLIGHT];
		GrowingArray<VkInternalMaterialDescriptorSet> m_materialDescriptors;


		VlkFrameBufferTexture* m_frameBufferTextures;
		uint32 numberOfFrameBufferTextures = 0;

		bool m_ownsRenderpass = true;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

		bool m_ownsFrameBuffer = true;
		VkFramebuffer m_frameBuffer[MAX_FRAMESINFLIGHT];
		
		VkDescriptorSetLayout m_materialSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_passSetLayout = VK_NULL_HANDLE;

		uint32 m_numberOfFramesInFlight = MAX_FRAMESINFLIGHT;

		void CleanupResource(VlkDevice& device);
	};

	struct VlkTextureData
	{
		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkImageView textureImageView = VK_NULL_HANDLE;
		void CleanupResource(VlkDevice& device);
	};
	struct VlkBufferObject
	{
		VkBuffer m_buffer[MAX_FRAMESINFLIGHT];
		VkDeviceMemory m_bufferMemory[MAX_FRAMESINFLIGHT];
		void* m_bufferMapped[MAX_FRAMESINFLIGHT];
		void CleanupResource(VlkDevice& device);
	};

}