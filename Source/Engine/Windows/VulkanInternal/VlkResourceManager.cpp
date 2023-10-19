#include "Engine_PCH.h"
#include "VlkResourceManager.h"
#include "Resources\TextureCommons.h"

#include "VlkDevice.h"
#include "VlkBufferCreationFunctions.h"
#include "VlkVertex_Descriptor.h"
#include "VlkSwapChain.h"
#include "Resources\Vulkan\VlkTextureManager.h"

#include "Containers\StaticArray\StaticArray.h"

namespace Hail
{
	bool VlkRenderingResourceManager::Init(RenderingDevice* renderingDevice, SwapChain* swapChain)
	{
		m_renderDevice = renderingDevice;
		m_swapChain = swapChain;
		VlkDevice& device = *(VlkDevice*)m_renderDevice;

		//initialize samplers
		m_resources.m_linearTextureSampler = CreateTextureSampler(device, TextureSamplerData{});
		TextureSamplerData pointSamplerData;
		pointSamplerData.sampler_mode = TEXTURE_SAMPLER_FILTER_MODE::POINT;
		m_resources.m_pointTextureSampler = CreateTextureSampler(device, pointSamplerData);

		//initialize buffers
		for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			volatile uint32_t index = 0;
			index = static_cast<uint32_t>(BUFFERS::TUTORIAL);
			if (!CreateBuffer(device, GetUniformBufferSize(BUFFERS::TUTORIAL), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_resources.m_buffers[index].m_buffer[i], m_resources.m_buffers[index].m_bufferMemory[i]))
			{
				return false;
			}
			vkMapMemory(device.GetDevice(), m_resources.m_buffers[index].m_bufferMemory[i], 0, GetUniformBufferSize(BUFFERS::TUTORIAL), 0, &m_resources.m_buffers[index].m_bufferMapped[i]);

			index = static_cast<uint32_t>(BUFFERS::SPRITE_INSTANCE_BUFFER);
			if (!CreateBuffer(device, GetUniformBufferSize(BUFFERS::SPRITE_INSTANCE_BUFFER), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_resources.m_buffers[index].m_buffer[i], m_resources.m_buffers[index].m_bufferMemory[i]))
			{
				return false;
			}
			vkMapMemory(device.GetDevice(), m_resources.m_buffers[index].m_bufferMemory[i], 0, GetUniformBufferSize(BUFFERS::SPRITE_INSTANCE_BUFFER), 0, &m_resources.m_buffers[index].m_bufferMapped[i]);

			index = static_cast<uint32_t>(BUFFERS::PER_FRAME_DATA);
			if (!CreateBuffer(device, GetUniformBufferSize(BUFFERS::PER_FRAME_DATA), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_resources.m_buffers[index].m_buffer[i], m_resources.m_buffers[index].m_bufferMemory[i]))
			{
				return false;
			}
			vkMapMemory(device.GetDevice(), m_resources.m_buffers[index].m_bufferMemory[i], 0, GetUniformBufferSize(BUFFERS::PER_FRAME_DATA), 0, &m_resources.m_buffers[index].m_bufferMapped[i]);
		}

		if (!SetUpCommonLayouts())
		{
			return false;
		}
	}

	void VlkRenderingResourceManager::ClearAllResources()
	{
		if (!m_renderDevice)
		{
			//TODO: Add a assert here
			return;
		}
		VlkDevice* device = (VlkDevice*)m_renderDevice;
		vkDestroySampler(device->GetDevice(), m_resources.m_linearTextureSampler, nullptr);
		vkDestroySampler(device->GetDevice(), m_resources.m_pointTextureSampler, nullptr);
		for (size_t i = 0; i < (uint32)(BUFFERS::COUNT); i++)
		{
			m_resources.m_buffers[i].CleanupResource(*device);
		}

		vkDestroyDescriptorSetLayout(device->GetDevice(), m_resources.m_globalPerFrameSetLayout, nullptr);
		vkDestroyDescriptorPool(device->GetDevice(), m_resources.m_globalDescriptorPool, nullptr);
	}

	void* VlkRenderingResourceManager::GetRenderingResources()
	{
		return &m_resources;
	}

	VkWriteDescriptorSet WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo& bufferInfo, uint32_t binding)
	{
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;

		write.dstBinding = binding;
		write.dstSet = dstSet;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pBufferInfo = &bufferInfo;
		return write;
	}

	void VlkRenderingResourceManager::MapMemoryToBuffer(BUFFERS buffer, void* dataToMap, uint32_t sizeOfData)
	{
		memcpy(m_resources.m_buffers[static_cast<uint32_t>(buffer)].m_bufferMapped[m_swapChain->GetFrameInFlight()], dataToMap, sizeOfData);
	}

	VkDescriptorSet& VlkRenderingResourceManager::GetGlobalDescriptorSet(uint32_t frameInFlight)
	{
		return m_resources.m_globalDescriptorSetsPerFrame[frameInFlight];
	}

	struct VlkLayoutDescriptor
	{
		VkShaderStageFlags flags;
		uint32_t bindingPoint;
		VkDescriptorType type;
	};

	bool CreateSetLayoutDescriptor(GrowingArray<VlkLayoutDescriptor> descriptors, VkDescriptorSetLayout& returnDescriptorLayoput, VlkDevice& device)
	{
		GrowingArray<VkDescriptorSetLayoutBinding>bindings;
		bindings.InitAndFill(descriptors.Size());

		for (uint32_t i = 0; i < descriptors.Size(); i++)
		{
			VkDescriptorSetLayoutBinding descriptor{};
			descriptor.binding = descriptors[i].bindingPoint;
			descriptor.descriptorCount = 1;
			descriptor.descriptorType = descriptors[i].type;
			descriptor.pImmutableSamplers = nullptr;
			descriptor.stageFlags = descriptors[i].flags;
			bindings[i] = descriptor;
		}

		VkDescriptorSetLayoutCreateInfo layoutSetInfo{};
		layoutSetInfo.bindingCount = bindings.Size();
		layoutSetInfo.flags = 0;
		layoutSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutSetInfo.pNext = nullptr;
		layoutSetInfo.pBindings = bindings.Data();
		if (vkCreateDescriptorSetLayout(device.GetDevice(), &layoutSetInfo, nullptr, &returnDescriptorLayoput) != VK_SUCCESS)
		{
			return false;
#ifdef DEBUG
			throw std::runtime_error("failed to create final pass descriptor set layout!");
#endif
		}
		return true;
	}

	bool VlkRenderingResourceManager::SetUpCommonLayouts()
	{
		VlkDevice& device = *(VlkDevice*)m_renderDevice;
		// SWIMMING POOOL 
		StaticArray<VkDescriptorPoolSize, 4> poolSizes =
		{
			{
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 }
			}
		};
		VkDescriptorPoolCreateInfo finalPoolInfo{};
		finalPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		finalPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.Getsize());
		finalPoolInfo.pPoolSizes = poolSizes.Data();
		finalPoolInfo.maxSets = 100;
		finalPoolInfo.flags = 0;
		if (vkCreateDescriptorPool(device.GetDevice(), &finalPoolInfo, nullptr, &m_resources.m_globalDescriptorPool) != VK_SUCCESS)
		{
			return false;
		}

		if (!CreateSetLayoutDescriptor({
			{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,  GetUniformBufferIndex(Hail::BUFFERS::PER_FRAME_DATA), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }
			}, m_resources.m_globalPerFrameSetLayout, device))
		{
			return false;
		}
		GrowingArray<VkDescriptorSetLayout> layouts(MAX_FRAMESINFLIGHT, m_resources.m_globalPerFrameSetLayout, false);
		VkDescriptorSetAllocateInfo passAllocInfo{};
		passAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		passAllocInfo.descriptorPool = m_resources.m_globalDescriptorPool;
		passAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMESINFLIGHT);
		passAllocInfo.pSetLayouts = layouts.Data();

		if (vkAllocateDescriptorSets(device.GetDevice(), &passAllocInfo, m_resources.m_globalDescriptorSetsPerFrame) != VK_SUCCESS)
		{
			return false;
#ifdef DEBUG
			throw std::runtime_error("failed to allocate descriptor sets!");
#endif
		}

		for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_resources.m_buffers[static_cast<uint32_t>(BUFFERS::PER_FRAME_DATA)].m_buffer[i];
			bufferInfo.offset = 0;
			bufferInfo.range = static_cast<VkDeviceSize>(GetUniformBufferSize(BUFFERS::PER_FRAME_DATA));
			VkWriteDescriptorSet perFrameBuffer = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_resources.m_globalDescriptorSetsPerFrame[i], bufferInfo, 0);
			VkWriteDescriptorSet setWrites[] = { perFrameBuffer };
			vkUpdateDescriptorSets(device.GetDevice(), 1, setWrites, 0, nullptr);
		}

		return true;
	}

}