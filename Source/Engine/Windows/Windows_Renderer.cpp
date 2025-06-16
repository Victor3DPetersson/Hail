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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "RenderCommands.h"

using namespace Hail;

bool Hail::VlkRenderer::Initialize()
{
	m_swapChain = (VlkSwapChain*)m_pResourceManager->GetSwapChain();
	Renderer::Initialize();

	////clear font textures from cpu data, so clearing ImGui for Vlk
	ImGui_ImplVulkan_DestroyFontUploadObjects();
	return true;
}

bool VlkRenderer::InitDevice(Timer* timer)
{
	m_timer = timer;
	m_renderDevice = new VlkDevice();
	m_renderDevice->CreateInstance();

	return true;
}

bool Hail::VlkRenderer::InitGraphicsEngineAndContext(ResourceManager* resourceManager)
{
	m_pResourceManager = resourceManager;
	m_pContext = new VlkRenderContext(m_renderDevice, m_pResourceManager);
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);

	return true;
}

void VlkRenderer::InitImGui()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
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

	if (vkCreateDescriptorPool(device.GetDevice(), &pool_info, nullptr, &m_imguiPool) != VK_SUCCESS)
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
	init_info.DescriptorPool = m_imguiPool;
	init_info.MinImageCount = MAX_FRAMESINFLIGHT;
	init_info.ImageCount = m_swapChain->GetSwapchainImageCount();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, m_swapChain->GetRenderPass());

	VlkCommandBuffer* pVlkCommandBfr = (VlkCommandBuffer*)m_pContext->GetCurrentCommandBuffer();
	VkCommandBuffer cmd = pVlkCommandBfr->m_commandBuffer;
	ImGui_ImplVulkan_CreateFontsTexture(cmd);
}

void Hail::VlkRenderer::WaitForGPU()
{
	H_ASSERT(GetIsMainThread(), "Only the main thread should wait for the GPU.");
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	vkDeviceWaitIdle(device.GetDevice());
}

void Hail::VlkRenderer::StartFrame(RenderCommandPool& renderPool)
{
	Renderer::StartFrame(renderPool);
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();
}

void VlkRenderer::Render()
{
	ImGui::Render();
	const uint32_t currentFrame = m_swapChain->GetFrameInFlight();
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
	VlkCommandBuffer* pVlkCommandBuffer = (VlkCommandBuffer*)m_pContext->GetCurrentCommandBuffer();
	VkCommandBuffer& commandBuffer = pVlkCommandBuffer->m_commandBuffer;
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void VlkRenderer::Cleanup()
{
	Renderer::Cleanup();

	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	vkDeviceWaitIdle(device.GetDevice());	  

	vkDestroyDescriptorPool(device.GetDevice(), m_imguiPool, nullptr);
	ImGui_ImplVulkan_Shutdown();

	m_pContext->Cleanup();
	m_pResourceManager->ClearAllResources(m_renderDevice);

	m_pSpriteVertexBuffer->CleanupResource(m_renderDevice);
	m_pVertexBuffer->CleanupResource(m_renderDevice);
	m_pIndexBuffer->CleanupResource(m_renderDevice);
	SAFEDELETE(m_pSpriteVertexBuffer);
	SAFEDELETE(m_pVertexBuffer);
	SAFEDELETE(m_pIndexBuffer);
	SAFEDELETE(m_pContext);

	vkDestroyCommandPool(device.GetDevice(), device.GetCommandPool(), nullptr);

	device.DestroyDevice();
}