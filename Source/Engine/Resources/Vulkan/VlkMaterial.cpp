#include "Engine_PCH.h"
#include "VlkMaterial.h"
#include "Windows\VulkanInternal\VlkDevice.h"

void Hail::VlkMaterial::CleanupResource(RenderingDevice& device)
{
	VlkDevice& vlkDevice = *(VlkDevice*)&device;
	//destruction functions of shared resources
	if (m_instanceSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(vlkDevice.GetDevice(), m_instanceSetLayout, nullptr);
		m_instanceSetLayout = VK_NULL_HANDLE;
	}
	for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
		CleanupResourceFrameData(device, i);
}

void Hail::VlkMaterial::CleanupResourceFrameData(RenderingDevice& device, uint32 frameInFlight)
{
}

Hail::VlkMaterialTypeObject::VlkMaterialTypeObject() :
	m_typeSetLayout(VK_NULL_HANDLE),
	m_globalSetLayout(VK_NULL_HANDLE)
{
	for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		m_typeDescriptors[i] = VK_NULL_HANDLE;
		m_globalDescriptors[i] = VK_NULL_HANDLE;
	}
}

void Hail::VlkMaterialTypeObject::CleanupResource(RenderingDevice& device)
{
	VlkDevice& vlkDevice = *(VlkDevice*)&device;
	if (m_typeSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(vlkDevice.GetDevice(), m_typeSetLayout, nullptr);
		m_typeSetLayout = VK_NULL_HANDLE;
	}
	if (m_globalSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(vlkDevice.GetDevice(), m_globalSetLayout, nullptr);
		m_globalSetLayout = VK_NULL_HANDLE;
	}
}

void Hail::VlkPipeline::CleanupResource(RenderingDevice& device)
{
	VlkDevice& vlkDevice = *(VlkDevice*)&device;
	if (m_pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(vlkDevice.GetDevice(), m_pipelineLayout, nullptr);
		m_pipelineLayout = VK_NULL_HANDLE;
	}
	if (m_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(vlkDevice.GetDevice(), m_pipeline, nullptr);
		m_pipeline = VK_NULL_HANDLE;
	}
	if (m_renderPass != VK_NULL_HANDLE)
	{
		if (m_ownsRenderpass)
		{
			vkDestroyRenderPass(vlkDevice.GetDevice(), m_renderPass, nullptr);
		}
		m_renderPass = VK_NULL_HANDLE;
	}
	m_frameBufferTextures = nullptr;
}

void Hail::VlkPipeline::CleanupResourceFrameData(RenderingDevice& device, uint32 frameInFlight)
{
	VlkDevice& vlkDevice = *(VlkDevice*)&device;
	if (m_frameBuffer[frameInFlight] != VK_NULL_HANDLE)
	{
		if (m_ownsFrameBuffer)
		{
			vkDestroyFramebuffer(vlkDevice.GetDevice(), m_frameBuffer[frameInFlight], nullptr);
		}
		m_frameBuffer[frameInFlight] = VK_NULL_HANDLE;
	}
}

void Hail::VlkMaterialPipeline::CleanupResource(RenderingDevice& device)
{
	m_pPipeline->CleanupResource(device);
	for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
		CleanupResourceFrameData(device, i);
}

void Hail::VlkMaterialPipeline::CleanupResourceFrameData(RenderingDevice& device, uint32 frameInFlight)
{
	m_pPipeline->CleanupResourceFrameData(device, frameInFlight);
}
