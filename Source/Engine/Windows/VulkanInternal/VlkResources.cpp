#include "Engine_PCH.h"
#include "VlkResources.h"

#include "DebugMacros.h"
#include "VlkDevice.h"

void Hail::VlkPassData::CleanupResource(VlkDevice& device)
{
	for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		if (m_frameBuffer[i] != VK_NULL_HANDLE)
		{
			if (m_ownsFrameBuffer)
			{
				vkDestroyFramebuffer(device.GetDevice(), m_frameBuffer[i], nullptr);
			}
			m_frameBuffer[i] = VK_NULL_HANDLE;
		}
	}
	if (m_materialSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device.GetDevice(), m_materialSetLayout, nullptr);
		m_materialSetLayout = VK_NULL_HANDLE;
	}
	if (m_passSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device.GetDevice(), m_passSetLayout, nullptr);
		m_passSetLayout = VK_NULL_HANDLE;
	}
	if (m_pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(device.GetDevice(), m_pipelineLayout, nullptr);
		m_pipelineLayout = VK_NULL_HANDLE;
	}
	if (m_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(device.GetDevice(), m_pipeline, nullptr);
		m_pipeline = VK_NULL_HANDLE;
	}
	if (m_renderPass != VK_NULL_HANDLE)
	{
		if (m_ownsRenderpass)
		{
			vkDestroyRenderPass(device.GetDevice(), m_renderPass, nullptr);
		}
		m_renderPass = VK_NULL_HANDLE;
	}
	m_renderPass = VK_NULL_HANDLE;
	m_frameBufferTextures = nullptr;
}

void Hail::VlkBufferObject::CleanupResource(VlkDevice& device)
{
	for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		if (m_buffer[i] != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device.GetDevice(), m_buffer[i], nullptr);
		}
		if (m_bufferMemory[i] != VK_NULL_HANDLE)
		{
			vkFreeMemory(device.GetDevice(), m_bufferMemory[i], nullptr);
		}
		m_buffer[i] = VK_NULL_HANDLE;
		m_bufferMemory[i] = VK_NULL_HANDLE;
	}
}
