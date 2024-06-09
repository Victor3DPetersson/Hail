#include "Engine_PCH.h"
#include "VlkResources.h"

#include "DebugMacros.h"
#include "VlkDevice.h"

void Hail::VlkPassData::CleanupResource(VlkDevice& device)
{
	//destruction functions of shared resources
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
	m_frameBufferTextures = nullptr;
}

void Hail::VlkPassData::CleanupResourceFrameData(VlkDevice& device, uint32 frameInFlight)
{
	if (m_frameBuffer[frameInFlight] != VK_NULL_HANDLE)
	{
		if (m_ownsFrameBuffer)
		{
			vkDestroyFramebuffer(device.GetDevice(), m_frameBuffer[frameInFlight], nullptr);
		}
		m_frameBuffer[frameInFlight] = VK_NULL_HANDLE;
	}
}
