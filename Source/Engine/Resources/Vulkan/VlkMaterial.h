#pragma once
#include "Resources\MaterialResources.h"
#include "Windows\VulkanInternal\VlkResources.h"

namespace Hail
{
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

	class VlkMaterialInstanceDescriptor : public MaterialInstanceDescriptor
	{
	public:
		VkInternalMaterialDescriptorSet m_instanceData;
	};

	class VlkMaterialTypeObject : public MaterialTypeObject
	{
	public:
		VlkMaterialTypeObject();

		void CleanupResource(RenderingDevice& device) override;

		VkDescriptorSetLayout m_typeSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_globalSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_typeDescriptors[MAX_FRAMESINFLIGHT];
		VkDescriptorSet m_globalDescriptors[MAX_FRAMESINFLIGHT];
	};

	class VlkPipeline : public Pipeline
	{
	public:

		void CleanupResource(RenderingDevice& device) override;
		void CleanupResourceFrameData(RenderingDevice& device, uint32 frameInFlight) override;
		VlkFrameBufferTexture* m_frameBufferTextures = nullptr;
		uint32 numberOfFrameBufferTextures = 0;

		bool m_ownsRenderpass = true;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

		bool m_ownsFrameBuffer = true;
		VkFramebuffer m_frameBuffer[MAX_FRAMESINFLIGHT];
	};

	class VlkMaterial : public Material
	{
	public:

		void CleanupResource(RenderingDevice& device) override;
		void CleanupResourceFrameData(RenderingDevice& device, uint32 frameInFlight) override;

		VkDescriptorSetLayout m_instanceSetLayout = VK_NULL_HANDLE;

		GrowingArray<VkInternalMaterialDescriptorSet> m_instanceDescriptors;


	};

	class VlkMaterialPipeline : public MaterialPipeline
	{
	public:
		void CleanupResource(RenderingDevice& device) override;
		void CleanupResourceFrameData(RenderingDevice& device, uint32 frameInFlight) override;

		
	};


}


