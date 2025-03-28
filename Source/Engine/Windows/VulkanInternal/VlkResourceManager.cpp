#include "Engine_PCH.h"
#include "VlkResourceManager.h"

#include "VlkDevice.h"
#include "VlkSwapChain.h"
#include "VlkTextureCreationFunctions.h"
#include "VlkVertex_Descriptor.h"
#include "Resources\Vulkan\VlkTextureManager.h"

#include "Containers\StaticArray\StaticArray.h"

#include "Resources\Vulkan\VlkBufferResource.h"

#include "vk_mem_alloc.h"

using namespace Hail;
bool VlkRenderingResourceManager::Init(RenderingDevice* renderingDevice, SwapChain* swapChain)
{
	m_renderDevice = renderingDevice;
	m_swapChain = swapChain;
	VlkDevice& device = *(VlkDevice*)m_renderDevice;

	// TODO: Move this out to a more clever spot where we can measure what wee register and stuff. 
	StaticArray<VkDescriptorPoolSize, 6> poolSizes =
	{
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100},
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 8 }
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

	return InternalInit();
}

void VlkRenderingResourceManager::ClearAllResources()
{
	H_ASSERT(m_renderDevice);


	for (uint32 i = 0; i < (uint32)GlobalSamplers::Count; i++)
	{
		m_samplers[i]->CleanupResource(m_renderDevice);
		SAFEDELETE(m_samplers[i]);
	}
	VlkDevice* device = (VlkDevice*)m_renderDevice;

	for (size_t iSet = 0; iSet < 2; iSet++)
	{
		for (size_t iBuffer = 0; iBuffer < m_uniformBuffers[iSet].Size(); iBuffer++)
		{
			m_uniformBuffers[iSet][iBuffer]->CleanupResource(m_renderDevice);
			SAFEDELETE(m_uniformBuffers[iSet][iBuffer]);
		}
		m_uniformBuffers[iSet].RemoveAll();
		for (size_t iBuffer = 0; iBuffer < m_structuredBuffers[iSet].Size(); iBuffer++)
		{
			m_structuredBuffers[iSet][iBuffer]->CleanupResource(m_renderDevice);
			SAFEDELETE(m_structuredBuffers[iSet][iBuffer]);
		}
		m_structuredBuffers[iSet].RemoveAll();
	}

	vkDestroyDescriptorPool(device->GetDevice(), m_resources.m_globalDescriptorPool, nullptr);
}

void* VlkRenderingResourceManager::GetRenderingResources()
{
	return &m_resources;
}

BufferObject* VlkRenderingResourceManager::CreateBuffer(BufferProperties properties)
{
	VlkBufferObject* vlkBuffer = new VlkBufferObject();
	if (vlkBuffer->Init(m_renderDevice, properties))
		return vlkBuffer;

	vlkBuffer->CleanupResource(m_renderDevice);
	return nullptr;
}

SamplerObject* Hail::VlkRenderingResourceManager::CreateSamplerObject(SamplerProperties properties)
{
	SamplerObject* sampler = new VlkSamplerObject();
	sampler->Init(m_renderDevice, properties);
	return sampler;
}
