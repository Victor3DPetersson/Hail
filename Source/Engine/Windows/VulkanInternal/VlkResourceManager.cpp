#include "Engine_PCH.h"
#include "VlkResourceManager.h"

#include "VlkDevice.h"
#include "VlkBufferCreationFunctions.h"
#include "VlkVertex_Descriptor.h"
#include "VlkSwapChain.h"
#include "Resources\Vulkan\VlkTextureManager.h"

#include "Containers\StaticArray\StaticArray.h"

#include "Resources\Vulkan\VlkBufferResource.h"

using namespace Hail;
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

	// TODO: Move this out to a more clever spot where we can measure what wee register and stuff. 
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

	return InternalInit();
}

void VlkRenderingResourceManager::ClearAllResources()
{
	if (!m_renderDevice)
	{
		//TODO: Add an assert here
		return;
	}
	VlkDevice* device = (VlkDevice*)m_renderDevice;
	vkDestroySampler(device->GetDevice(), m_resources.m_linearTextureSampler, nullptr);
	vkDestroySampler(device->GetDevice(), m_resources.m_pointTextureSampler, nullptr);
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

void VlkRenderingResourceManager::MapMemoryToBuffer(BufferObject* pBuffer, void* dataToMap, uint32_t sizeOfData)
{
	// TODO assert here on pBuffer
	VlkBufferObject* pVlkBuffer = (VlkBufferObject*)pBuffer;
	// TODO: Assert on size of data on the properties of the buffer
	memcpy(pVlkBuffer->GetMappedMemory(m_swapChain->GetFrameInFlight()), dataToMap, sizeOfData);
}

void VlkRenderingResourceManager::CreateBuffer(BufferProperties properties, eDecorationSets setToCreateBufferFor)
{
	// TODO assert on type::
	VlkBufferObject* vlkBuffer = new VlkBufferObject();
	vlkBuffer->Init(m_renderDevice, properties);
	// TODO: add error on failure of buffer creation
	if (properties.type == eBufferType::structured)
	{
		m_structuredBuffers[setToCreateBufferFor].Add(vlkBuffer);
	}
	else if (properties.type == eBufferType::uniform)
	{
		m_uniformBuffers[setToCreateBufferFor].Add(vlkBuffer);
	}
}
