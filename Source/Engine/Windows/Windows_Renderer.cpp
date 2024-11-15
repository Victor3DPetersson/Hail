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
#include "VulkanInternal/VlkBufferCreationFunctions.h"
#include "VulkanInternal/VlkSingleTimeCommand.h"
#include "VulkanInternal/VlkFrameBufferTexture.h"
#include "Resources\Vulkan\VlkMaterial.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "RenderCommands.h"

using namespace Hail;

bool VlkRenderer::InitDevice(Timer* timer)
{
	m_timer = timer;
	m_renderDevice = new VlkDevice();
	m_renderDevice->CreateInstance();
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);

	return true;
}

bool Hail::VlkRenderer::InitGraphicsEngine(ResourceManager* resourceManager)
{
	m_resourceManager = resourceManager;
	m_swapChain = (VlkSwapChain*)m_resourceManager->GetSwapChain();

	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);


	CreateFullscreenVertexBuffer();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateDebugLineVertexBuffer();
	CreateCommandBuffers();
	CreateSyncObjects();
	InitImGui();

	CreateSpriteVertexBuffer();

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

	VkCommandBuffer cmd = BeginSingleTimeCommands(device, device.GetCommandPool());

	ImGui_ImplVulkan_CreateFontsTexture(cmd);
	////execute a gpu command to upload imgui font textures
	//immediate_submit([&](VkCommandBuffer cmd) {
	//	ImGui_ImplVulkan_CreateFontsTexture(cmd);
	//	});
	EndSingleTimeCommands(device, cmd, device.GetGraphicsQueue(), device.GetCommandPool());
	////clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();


	////add the destroy the imgui created structures
	//_mainDeletionQueue.push_function([=]() {

	//	vkDestroyDescriptorPool(_device, imguiPool, nullptr);
	//	ImGui_ImplVulkan_Shutdown();
	//	});
}

void Hail::VlkRenderer::WaitForGPU()
{
	H_ASSERT(GetIsMainThread(), "Only the main thread should wait for the GPU.");
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	vkDeviceWaitIdle(device.GetDevice());
}

void Hail::VlkRenderer::StartFrame(RenderCommandPool& renderPool)
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	if (m_swapChain->FrameStart(device, m_inFrameFences, m_imageAvailableSemaphores))
	{
		//Swapchain has been resized
	}
	Renderer::StartFrame(renderPool);
	m_framebufferResized = false;
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplVulkan_NewFrame();

	ImGui::NewFrame();
	//g_camera = renderPool.renderCamera;
}

void VlkRenderer::Render()
{
	ImGui::Render();
	const uint32_t currentFrame = m_swapChain->GetFrameInFlight();
	vkResetCommandBuffer(m_commandBuffers[currentFrame], 0);
	Renderer::Render();

	//RecordCommandBuffer(m_commandBuffers[currentFrame], m_swapChain->GetCurrentSwapImageIndex());
}


void Hail::VlkRenderer::EndFrame()
{
	Renderer::EndFrame();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	const uint32_t currentFrame = m_swapChain->GetFrameInFlight();
	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[currentFrame] };

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	if (vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, m_inFrameFences[currentFrame]) != VK_SUCCESS)
	{
#ifdef DEBUG
		//throw std::runtime_error("failed to submit draw command buffer!");
#endif
	}
	m_swapChain->FrameEnd(signalSemaphores, device.GetPresentQueue());
	m_boundMaterialType = eMaterialType::COUNT;
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

void Hail::VlkRenderer::BindMaterialPipeline(Pipeline* pPipelineToBind, bool bFirstMaterialInFrame)
{
	H_ASSERT(pPipelineToBind, "Material bound without a valid pipeline.");

	if (m_currentlyBoundPipeline != pPipelineToBind->m_sortKey && !bFirstMaterialInFrame)
	{
		EndMaterialPass();
	}
	if (m_currentlyBoundPipeline == pPipelineToBind->m_sortKey)
	{
		return;
	}
	m_currentlyBoundPipeline = pPipelineToBind->m_sortKey;

	const uint32_t frameInFlightIndex = m_swapChain->GetFrameInFlight();
	MaterialTypeObject* pMaterialTypeObject = pPipelineToBind->m_pTypeDescriptor;
	VkCommandBuffer& commandBuffer = m_commandBuffers[frameInFlightIndex];
	VlkPipeline* pVlkPipeline = (VlkPipeline*)pPipelineToBind;
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (!m_commandBufferBound)
	{
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
#ifdef DEBUG
			throw std::runtime_error("failed to begin recording command buffer!");
#endif
		}
		m_commandBufferBound = true;
	}

	const glm::uvec2 passResolution = pVlkPipeline->m_frameBufferTextures->GetResolution();
	VkExtent2D extent = { passResolution.x, passResolution.y };
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	//TEMP: Will remove once we set up reload from resource manager
	if (pVlkPipeline->m_type != eMaterialType::FULLSCREEN_PRESENT_LETTERBOX)
	{
		renderPassInfo.renderPass = pVlkPipeline->m_renderPass;
		renderPassInfo.framebuffer = pVlkPipeline->m_frameBuffer[frameInFlightIndex];
	}
	else
	{
		renderPassInfo.renderPass = m_swapChain->GetRenderPass();
		renderPassInfo.framebuffer = m_swapChain->GetFrameBuffer(m_swapChain->GetCurrentSwapImageIndex());
	}
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	const glm::vec3 mainClearColor = pVlkPipeline->m_frameBufferTextures->GetClearColor();
	VkClearValue mainClearColors[2];
	mainClearColors[0].color = { mainClearColor.x, mainClearColor.y, mainClearColor.z, 1.0f };
	mainClearColors[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = mainClearColors;
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)passResolution.x;
	viewport.height = (float)passResolution.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D mainScissor{};
	mainScissor.offset = { 0, 0 };
	mainScissor.extent = extent;
	vkCmdSetScissor(commandBuffer, 0, 1, &mainScissor);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pVlkPipeline->m_pipeline);

	if (pVlkPipeline->m_type == eMaterialType::CUSTOM)
	{
		VectorOnStack< VkDescriptorSet, 3> descriptorSets = localGetMaterialDescriptors(m_resourceManager, pVlkPipeline, frameInFlightIndex);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pVlkPipeline->m_pipelineLayout, 0, descriptorSets.Size(), descriptorSets.Data(), 0, nullptr);
		m_boundMaterialType = eMaterialType::CUSTOM;
	}

}

void Hail::VlkRenderer::EndMaterialPass()
{
	const uint32_t frameInFlightIndex = m_swapChain->GetFrameInFlight();
	vkCmdEndRenderPass(m_commandBuffers[frameInFlightIndex]);
}

void Hail::VlkRenderer::RenderSprite(const RenderCommand_Sprite& spriteCommandToRender, uint32_t spriteInstance)
{
	const MaterialInstance& materialInstance = m_resourceManager->GetMaterialManager()->GetMaterialInstance(m_commandPoolToRender->spriteCommands[spriteInstance].materialInstanceID, eMaterialType::SPRITE);
	BindMaterialPipeline(m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::SPRITE, materialInstance.m_materialIndex)->m_pPipeline, false);

	const uint32_t frameInFlightIndex = m_swapChain->GetFrameInFlight();
	VkCommandBuffer& commandBuffer = m_commandBuffers[frameInFlightIndex];
	VlkMaterial& vlkMaterial = *(VlkMaterial*)m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::SPRITE, materialInstance.m_materialIndex);
	VlkPipeline& vlkPipeline = *(VlkPipeline*)vlkMaterial.m_pPipeline;

	//binding pass data first round
	if (m_boundMaterialType != eMaterialType::SPRITE)
	{
		VectorOnStack< VkDescriptorSet, 3> descriptorSets = localGetMaterialDescriptors(m_resourceManager, &vlkPipeline, frameInFlightIndex);
		VkDeviceSize spriteOffsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_spriteVertexBuffer, spriteOffsets);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vlkPipeline.m_pipelineLayout, 0, descriptorSets.Size(), descriptorSets.Data(), 0, nullptr);
		m_boundMaterialType = eMaterialType::SPRITE;
	}

	// TODO: use the reflected push constants
	glm::uvec4 pushConstants_instanceID_padding = { spriteInstance, 0, 0, 0 };
	const MaterialInstance& instanceMaterialData = m_resourceManager->GetMaterialManager()->GetMaterialInstance(spriteCommandToRender.materialInstanceID, eMaterialType::SPRITE);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vlkPipeline.m_pipelineLayout, 2, 1, &vlkMaterial.m_instanceDescriptors[instanceMaterialData.m_gpuResourceInstance].descriptors[frameInFlightIndex], 0, nullptr);
	vkCmdPushConstants(commandBuffer, vlkPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::uvec4), &pushConstants_instanceID_padding);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

void Hail::VlkRenderer::RenderMesh(const RenderCommand_Mesh& meshCommandToRender, uint32_t meshInstance)
{
	const uint32_t frameInFlightIndex = m_swapChain->GetFrameInFlight();
	VkCommandBuffer& commandBuffer = m_commandBuffers[frameInFlightIndex];
	VlkMaterial& material = *(VlkMaterial*)m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::MODEL3D, 0);
	VlkPipeline& vlkPipeline = *(VlkPipeline*)material.m_pPipeline;

	//binding pass data first round
	if (m_boundMaterialType != eMaterialType::MODEL3D)
	{
		VectorOnStack< VkDescriptorSet, 3> descriptorSets = localGetMaterialDescriptors(m_resourceManager, &vlkPipeline, frameInFlightIndex);
		if (material.m_instanceSetLayout != VK_NULL_HANDLE)
		{
			descriptorSets.Add(material.m_instanceDescriptors[0].descriptors[frameInFlightIndex]);
		}
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vlkPipeline.m_pipelineLayout, 0, descriptorSets.Size(), descriptorSets.Data(), 0, nullptr);
		m_boundMaterialType = eMaterialType::MODEL3D;
	}

	//Todo: Make sure to fix vertex binding and that kind of stuff later and make the resources be held outside of renderer
	VkBuffer mainVertexBuffers[] = { m_vertexBuffer };
	VkDeviceSize mainOffsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, mainVertexBuffers, mainOffsets);
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_resourceManager->m_unitCube.indices.Size()), 1, 0, 0, 0);
}

void Hail::VlkRenderer::RenderDebugLines2D(uint32 numberOfLinesToRender, uint32 offsetFrom3DLines)
{
	const uint32_t frameInFlightIndex = m_swapChain->GetFrameInFlight();
	VkCommandBuffer& commandBuffer = m_commandBuffers[frameInFlightIndex];
	VlkMaterial& vlkMaterial = *(VlkMaterial*)m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::DEBUG_LINES2D, 0);
	VlkPipeline& vlkPipeline = *(VlkPipeline*)vlkMaterial.m_pPipeline;

	//binding pass data first round
	if (m_boundMaterialType != eMaterialType::DEBUG_LINES2D)
	{
		VectorOnStack< VkDescriptorSet, 3> descriptorSets = localGetMaterialDescriptors(m_resourceManager, &vlkPipeline, frameInFlightIndex);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vlkPipeline.m_pipelineLayout, 0, descriptorSets.Size(), descriptorSets.Data(), 0, nullptr);
		m_boundMaterialType = eMaterialType::DEBUG_LINES2D;
	}

	VkBuffer mainVertexBuffers[] = { m_debugLineVertexBuffer };
	VkDeviceSize mainOffsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, mainVertexBuffers, mainOffsets);

	vkCmdDraw(commandBuffer, numberOfLinesToRender, 1, offsetFrom3DLines, 0);
}

void Hail::VlkRenderer::RenderDebugLines3D(uint32 numberOfLinesToRender)
{
	//TODO:
}

void Hail::VlkRenderer::RenderLetterBoxPass()
{
	const uint32 frameInFlightIndex = m_swapChain->GetFrameInFlight();
	VkCommandBuffer& commandBuffer = m_commandBuffers[frameInFlightIndex];
	VlkMaterial& vlkMaterial = *(VlkMaterial*)m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::FULLSCREEN_PRESENT_LETTERBOX, 0);
	VlkPipeline& vlkPipeline = *(VlkPipeline*)vlkMaterial.m_pPipeline;

	//binding pass data first round
	if (m_boundMaterialType != eMaterialType::FULLSCREEN_PRESENT_LETTERBOX)
	{
		VectorOnStack< VkDescriptorSet, 3> descriptorSets = localGetMaterialDescriptors(m_resourceManager, &vlkPipeline, frameInFlightIndex);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vlkPipeline.m_pipelineLayout, 0, descriptorSets.Size(), descriptorSets.Data(), 0, nullptr);
		m_boundMaterialType = eMaterialType::FULLSCREEN_PRESENT_LETTERBOX;
	}

	VkBuffer vertexBuffers[] = { m_fullscreenVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to record command buffer!");
#endif
	}
	m_commandBufferBound = false;
}

void Hail::VlkRenderer::RenderMeshlets(glm::uvec3 dispatchSize)
{
	const uint32 frameInFlightIndex = m_swapChain->GetFrameInFlight();
	VkCommandBuffer& commandBuffer = m_commandBuffers[frameInFlightIndex];

	vkCmdDrawMeshTasksEXT(commandBuffer, dispatchSize.x, dispatchSize.y, dispatchSize.z);
}


void VlkRenderer::Cleanup()
{
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	vkDeviceWaitIdle(device.GetDevice());	  

	vkDestroyDescriptorPool(device.GetDevice(), m_imguiPool, nullptr);
	ImGui_ImplVulkan_Shutdown();

	m_resourceManager->ClearAllResources();
	m_swapChain->DestroySwapChain((VlkDevice*)(m_renderDevice));

	vkDestroyBuffer(device.GetDevice(), m_fullscreenVertexBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), m_fullscreenVertexBufferMemory, nullptr);

	vkDestroyBuffer(device.GetDevice(), m_spriteVertexBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), m_spriteVertexBufferMemory, nullptr);

	vkDestroyBuffer(device.GetDevice(), m_debugLineVertexBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), m_debugLineVertexBufferMemory, nullptr);

	vkDestroyBuffer(device.GetDevice(), m_indexBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), m_indexBufferMemory, nullptr);
	vkDestroyBuffer(device.GetDevice(), m_vertexBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), m_vertexBufferMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		vkDestroySemaphore(device.GetDevice(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device.GetDevice(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device.GetDevice(), m_inFrameFences[i], nullptr);
	}
	vkDestroyCommandPool(device.GetDevice(), device.GetCommandPool(), nullptr);

	device.DestroyDevice();
}

void VlkRenderer::CreateCommandBuffers()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = device.GetCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = MAX_FRAMESINFLIGHT;

	if (vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, m_commandBuffers) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to allocate command buffers!");
#endif
	}
}

void VlkRenderer::CreateSyncObjects()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++) 
	{
		if (vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device.GetDevice(), &fenceInfo, nullptr, &m_inFrameFences[i]) != VK_SUCCESS) {
#ifdef DEBUG
			throw std::runtime_error("failed to create synchronization objects for a frame!");
#endif
		}
	}
}

void VlkRenderer::CreateVertexBuffer()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkDeviceSize bufferSize = sizeof(VertexModel) * m_resourceManager->m_unitCube.vertices.Size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_resourceManager->m_unitCube.vertices.Data(), (size_t)bufferSize);
	vkUnmapMemory(device.GetDevice(), stagingBufferMemory);

	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		m_vertexBuffer, m_vertexBufferMemory);

	CopyBuffer(device, stagingBuffer, m_vertexBuffer, bufferSize, device.GetGraphicsQueue(), device.GetCommandPool());
	vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);
}

void Hail::VlkRenderer::CreateFullscreenVertexBuffer()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkDeviceSize bufferSize = sizeof(uint32_t) * 3;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);
	uint32_t vertices[3] = { 0, 1, 2 };
	void* data;
	vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices, (size_t)bufferSize);
	vkUnmapMemory(device.GetDevice(), stagingBufferMemory);

	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_fullscreenVertexBuffer, m_fullscreenVertexBufferMemory);

	CopyBuffer(device, stagingBuffer, m_fullscreenVertexBuffer, bufferSize, device.GetGraphicsQueue(), device.GetCommandPool());
	vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);

}

void Hail::VlkRenderer::CreateSpriteVertexBuffer()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkDeviceSize spriteBufferSize = sizeof(uint32_t) * 6;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(device, spriteBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);
	uint32_t vertices[6] = { 0, 1, 2, 3, 4, 5 };
	void* data;
	vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, spriteBufferSize, 0, &data);
	memcpy(data, vertices, (size_t)spriteBufferSize);
	vkUnmapMemory(device.GetDevice(), stagingBufferMemory);

	CreateBuffer(device, spriteBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_spriteVertexBuffer, m_spriteVertexBufferMemory);

	CopyBuffer(device, stagingBuffer, m_spriteVertexBuffer, spriteBufferSize, device.GetGraphicsQueue(), device.GetCommandPool());
	vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);
}

void VlkRenderer::CreateIndexBuffer()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkDeviceSize bufferSize = sizeof(uint32_t) * m_resourceManager->m_unitCube.indices.Size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_resourceManager->m_unitCube.indices.Data(), (size_t)bufferSize);
	vkUnmapMemory(device.GetDevice(), stagingBufferMemory);

	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_indexBuffer, m_indexBufferMemory);

	CopyBuffer(device, stagingBuffer, m_indexBuffer, bufferSize, device.GetGraphicsQueue(), device.GetCommandPool());

	vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);
}

void Hail::VlkRenderer::CreateDebugLineVertexBuffer()
{
	GrowingArray<uint32> debugLineVertices(MAX_NUMBER_OF_DEBUG_LINES, 0);
	for (uint32 i = 0; i < MAX_NUMBER_OF_DEBUG_LINES; i++)
	{
		debugLineVertices[i] = i;
	}

	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	const VkDeviceSize bufferSize = sizeof(uint32_t) * MAX_NUMBER_OF_DEBUG_LINES;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, debugLineVertices.Data(), (size_t)bufferSize);
	vkUnmapMemory(device.GetDevice(), stagingBufferMemory);

	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_debugLineVertexBuffer, m_debugLineVertexBufferMemory);

	CopyBuffer(device, stagingBuffer, m_debugLineVertexBuffer, bufferSize, device.GetGraphicsQueue(), device.GetCommandPool());
	vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);
}
