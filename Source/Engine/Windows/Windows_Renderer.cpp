#include "Engine_PCH.h"
#include "Windows_Renderer.h"
#include "Utilities.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

#include <array>
#include <set>
#include <stdio.h>

#include "MathUtils.h"

#include "DebugMacros.h"

#include "Windows_ApplicationWindow.h"
#include "HailEngine.h"

#include "Resources\Vertices.h"
#include "Timer.h"
#include "Resources\ResourceManager.h"
#include "Resources\Vulkan\VlkMaterialManager.h"

#include "VulkanInternal\VlkSwapChain.h"
#include "VulkanInternal\VlkResourceManager.h"

#include "VulkanInternal/VlkVertex_Descriptor.h"
#include "VulkanInternal/VlkTextureCreationFunctions.h"
#include "VulkanInternal/VlkFrameBufferTexture.h"
#include "VulkanInternal/VlkRenderContext.h"
#include "Resources\Vulkan\VlkMaterial.h"
#include "Resources\Vulkan\VlkBufferResource.h"
#include "Settings.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "RenderCommands.h"

using namespace Hail;

void Hail::VlkRenderer::Initialize(ErrorManager* pErrorManager)
{
	m_swapChain = (VlkSwapChain*)m_pResourceManager->GetSwapChain();

	VlkDevice& device = *(VlkDevice*)m_renderDevice;



	Renderer::Initialize(pErrorManager);
}

void VlkRenderer::InitDevice(Timer* pTimer, ErrorManager* pErrorManager)
{
	m_timer = pTimer;
	m_renderDevice = new VlkDevice();
	m_renderDevice->CreateInstance(pErrorManager);
}

void Hail::VlkRenderer::InitGraphicsEngineAndContext(ResourceManager* resourceManager, ErrorManager* pErrorManager)
{
	m_pResourceManager = resourceManager;
	RenderContextStartupParams contextStartUpParams{};
	contextStartUpParams.pDevice = m_renderDevice;
	contextStartUpParams.pResourceManager = m_pResourceManager;
	contextStartUpParams.pErrorManager = pErrorManager;

	m_pContext = new VlkRenderContext(contextStartUpParams);
}

void VlkRenderer::InitImGui()
{
	if (!m_renderFrameSettings.m_bEnableImgui)
		return;

	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = *device.GetCommandPool(0);
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, &m_imguiVkData.m_commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//vkResetCommandBuffer(m_commandBuffer, 0);

	vkBeginCommandBuffer(m_imguiVkData.m_commandBuffer, &beginInfo);

	//1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool(device.GetDevice(), &pool_info, nullptr, &m_imguiVkData.m_imguiPool) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create ImGuiDescriptor");
#endif
	}
	// 2: initialize imgui library
	//this initializes the core structures of imgui
	ImGui::CreateContext();

	Windows_ApplicationWindow* appWindow = reinterpret_cast<Windows_ApplicationWindow*> (Hail::GetApplicationWIndow());
	if (!appWindow)
	{
#ifdef DEBUG
		throw std::runtime_error("No Window Handle, how did we get here?");
#endif
	}
	ImGui_ImplWin32_Init(appWindow->GetWindowHandle());
	ImGui::StyleColorsDark();

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = device.GetInstance();
	init_info.PhysicalDevice = device.GetPhysicalDevice();
	init_info.Device = device.GetDevice();
	init_info.Queue = device.GetGraphicsQueue();
	init_info.DescriptorPool = m_imguiVkData.m_imguiPool;
	init_info.MinImageCount = MAX_FRAMESINFLIGHT;
	init_info.ImageCount = m_swapChain->GetSwapchainImageCount();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, m_swapChain->GetRenderPass());

	VlkCommandBuffer* pVlkCommandBfr = (VlkCommandBuffer*)m_pContext->GetCurrentCommandBuffer();
	VkCommandBuffer cmd = m_imguiVkData.m_commandBuffer;
	ImGui_ImplVulkan_CreateFontsTexture(cmd);

	vkEndCommandBuffer(cmd);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.GetGraphicsQueue());

	vkFreeCommandBuffers(device.GetDevice(), *device.GetCommandPool(0), 1, &cmd);
	ImGui_ImplVulkan_DestroyFontUploadObjects();

}

void Hail::VlkRenderer::WaitForGPU()
{
	H_ASSERT(GetIsMainThread(), "Only the main thread should wait for the GPU.");
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	vkDeviceWaitIdle(device.GetDevice());
}

void Hail::VlkRenderer::StartFrame(RenderStartFrameParams startParams)
{
	Renderer::StartFrame(startParams);
	if (m_renderFrameSettings.m_bEnableImgui)
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();
	}
}

void VlkRenderer::Render()
{
	if (m_renderFrameSettings.m_bEnableImgui)
		ImGui::Render();

	Renderer::Render();
}

VectorOnStack< VkDescriptorSet, 3> localGetMaterialDescriptors(ResourceManager* pResourceManager, VlkPipeline* pVlkPipeline, uint32 frameInFlightIndex)
{
	VectorOnStack< VkDescriptorSet, 3> descriptorSets;
	VlkMaterialTypeObject* pMaterialType = (VlkMaterialTypeObject*)pResourceManager->GetMaterialManager()->GetTypeData(pVlkPipeline);;

	if (pMaterialType->m_globalSetLayout != VK_NULL_HANDLE)
	{
		descriptorSets.Add(pMaterialType->m_globalDescriptors[frameInFlightIndex]);
	}
	if (pMaterialType->m_typeSetLayout != VK_NULL_HANDLE)
	{
		descriptorSets.Add(pMaterialType->m_typeDescriptors[frameInFlightIndex]);
	}

	return descriptorSets;
}

void Hail::VlkRenderer::RenderMesh(const RenderData_Mesh& meshCommandToRender, uint32_t meshInstance)
{
	const uint32_t frameInFlightIndex = m_swapChain->GetFrameInFlight();
	VlkCommandBuffer* pVlkCommandBfr = (VlkCommandBuffer*)m_pContext->GetCurrentCommandBuffer();
	VkCommandBuffer commandBuffer = pVlkCommandBfr->m_commandBuffer;

	VlkMaterial& vlkMaterial = *(VlkMaterial*)m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::MODEL3D, 0);
	VlkPipeline& vlkPipeline = *(VlkPipeline*)vlkMaterial.m_pPipeline;

	// TODO: move to the context a bind function for material instance data
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vlkPipeline.m_pipelineLayout, 2, 1, &vlkMaterial.m_instanceDescriptors[0].descriptors[frameInFlightIndex], 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_pResourceManager->m_unitCube.indices.Size()), 1, 0, 0, 0);
}

void Hail::VlkRenderer::RenderImGui()
{
	if (!m_renderFrameSettings.m_bEnableImgui)
		return;

	VlkCommandBuffer* pVlkCommandBuffer = (VlkCommandBuffer*)m_pContext->GetCurrentCommandBuffer();
	VkCommandBuffer& commandBuffer = pVlkCommandBuffer->m_commandBuffer;
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void VlkRenderer::Cleanup()
{
	Renderer::Cleanup();

	if (!m_renderDevice)
		return;

	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	vkDeviceWaitIdle(device.GetDevice());	  

	if (m_renderFrameSettings.m_bEnableImgui)
	{
		vkDestroyDescriptorPool(device.GetDevice(), m_imguiVkData.m_imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	}


	m_pContext->Cleanup();
	m_pResourceManager->ClearAllResources(m_renderDevice);

	if (m_pSpriteVertexBuffer)
		m_pSpriteVertexBuffer->CleanupResource(m_renderDevice);
	if (m_pSpriteVertexBuffer)
		m_pVertexBuffer->CleanupResource(m_renderDevice);
	if (m_pSpriteVertexBuffer)
		m_pIndexBuffer->CleanupResource(m_renderDevice);
	SAFEDELETE(m_pSpriteVertexBuffer);
	SAFEDELETE(m_pVertexBuffer);
	SAFEDELETE(m_pIndexBuffer);
	SAFEDELETE(m_pContext);

	for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		vkDestroyCommandPool(device.GetDevice(), *device.GetCommandPool(i), nullptr);
	}

	device.DestroyDevice();
}