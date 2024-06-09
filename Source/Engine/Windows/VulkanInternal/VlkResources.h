#pragma once

#include <Windows.h>
#include "VlkFrameBufferTexture.h"
#include "Types.h"

namespace Hail
{
	class VlkDevice;
	class VlkPassData
	{
	public:
		VlkPassData()
		{
			for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
			{
				m_passDescriptors[i] = VK_NULL_HANDLE;
				m_frameBuffer[i] = VK_NULL_HANDLE;
			}
		}
		struct VkInternalMaterialDescriptorSet
		{
			VkInternalMaterialDescriptorSet()
			{
				for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
				{
					descriptors[i] = VK_NULL_HANDLE;
				}
			}
			VkDescriptorSet descriptors[MAX_FRAMESINFLIGHT];
		};

		void CleanupResource(VlkDevice& device);
		void CleanupResourceFrameData(VlkDevice& device, uint32 frameInFlight);

		GrowingArray<VkInternalMaterialDescriptorSet> m_materialDescriptors;
		VkDescriptorSet m_passDescriptors[MAX_FRAMESINFLIGHT];


		VlkFrameBufferTexture* m_frameBufferTextures = nullptr;
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

	};



	struct VlkTextureData
	{
		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkImageView textureImageView = VK_NULL_HANDLE;
	};

	//struct VlkBufferObject
	//{
	//	VkBuffer m_buffer[MAX_FRAMESINFLIGHT];
	//	VkDeviceMemory m_bufferMemory[MAX_FRAMESINFLIGHT];
	//	void* m_bufferMapped[MAX_FRAMESINFLIGHT];
	//	
	//};

}