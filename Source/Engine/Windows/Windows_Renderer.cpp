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
#include "Rendering\UniformBufferManager.h"

#include "Resources\Vertices.h"
#include "Timer.h"
#include "Resources\ResourceManager.h"

#include "VulkanInternal/VlkVertex_Descriptor.h"
#include "VulkanInternal/VlkTextureCreationFunctions.h"
#include "VulkanInternal/VlkBufferCreationFunctions.h"
#include "VulkanInternal/VlkSingleTimeCommand.h"
#include "VulkanInternal/VlkFrameBufferTexture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "RenderCommands.h"

using namespace Hail;


bool VlkRenderer::InitDevice(RESOLUTIONS startupResolution, Timer* timer)
{
	m_timer = timer;
	m_renderDevice = new VlkDevice();
	m_renderDevice->CreateInstance();
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);

	m_swapChain.Init(device);
	const VkExtent2D swapChainExtent = m_swapChain.GetSwapChainExtent();
	CalculateRenderResolution({ swapChainExtent.width, swapChainExtent.height });



	return true;
}
bool Hail::VlkRenderer::InitGraphicsEngine(ResourceManager* resourceManager)
{
	m_resourceManager = resourceManager;
	m_mainPassFrameBufferTexture = FrameBufferTexture_Create("MainRenderPass", m_renderTargetResolution, TEXTURE_FORMAT::R8G8B8A8_UINT, TEXTURE_DEPTH_FORMAT::D16_UNORM);

	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();

	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	m_textureSampler = CreateTextureSampler(device, TextureSamplerData{});
	TextureSamplerData pointSamplerData;
	pointSamplerData.sampler_mode = TEXTURE_SAMPLER_FILTER_MODE::POINT;
	m_pointTextureSampler = CreateTextureSampler(device, pointSamplerData);

	CreateFullscreenVertexBuffer();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSyncObjects();
	InitImGui();


	CreateMainDescriptorSetLayout();
	CreateMainDescriptorPool();
	CreateMainDescriptorSets();

	CreateMainRenderPass();
	CreateMainFrameBuffer();
	CreateMainGraphicsPipeline();

	CreateSpriteVertexBuffer();

	InitGlobalDescriptors();

	CreateSpriteDescriptorSets();
	CreateSpriteRenderPass();
	CreateSpriteGraphicsPipeline();
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
	init_info.ImageCount = m_swapChain.GetSwapchainImageCount();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, m_swapChain.GetRenderPass());

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

namespace
{
	Hail::Camera g_camera;
}



void Hail::VlkRenderer::StartFrame(RenderCommandPool& renderPool)
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	Renderer::StartFrame(renderPool);
	if (m_swapChain.FrameStart(device, m_inFrameFences, m_imageAvailableSemaphores, m_framebufferResized))
	{
		const VkExtent2D swapChainExtent = m_swapChain.GetSwapChainExtent();
		CalculateRenderResolution({ swapChainExtent.width, swapChainExtent.height });
	}
	m_framebufferResized = false;
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();
	g_camera = renderPool.renderCamera;



}

void VlkRenderer::Render()
{
	ImGui::Render();
	const uint32_t currentFrame = m_swapChain.GetCurrentFrame();
	vkResetCommandBuffer(m_commandBuffers[currentFrame], 0);
	UpdateUniformBuffer(currentFrame);
	RecordCommandBuffer(m_commandBuffers[currentFrame], m_swapChain.GetCurrentSwapImageIndex());
}


void Hail::VlkRenderer::EndFrame()
{
	Renderer::EndFrame();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	const uint32_t currentFrame = m_swapChain.GetCurrentFrame();
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
		throw std::runtime_error("failed to submit draw command buffer!");
#endif
	}
	m_swapChain.FrameEnd(signalSemaphores, device.GetPresentQueue());
}

void VlkRenderer::Cleanup()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	vkDeviceWaitIdle(device.GetDevice());
	m_swapChain.DestroySwapChain(device);

	vkDestroyDescriptorPool(device.GetDevice(), m_imguiPool, nullptr);
	ImGui_ImplVulkan_Shutdown();

	vkDestroySampler(device.GetDevice(), m_textureSampler, nullptr);
	vkDestroySampler(device.GetDevice(), m_pointTextureSampler, nullptr);

	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++) 
	{
		vkDestroyBuffer(device.GetDevice(), m_uniformBuffers[i], nullptr);
		vkFreeMemory(device.GetDevice(), m_uniformBuffersMemory[i], nullptr);

		vkDestroyBuffer(device.GetDevice(), m_perFrameDataBuffers[i], nullptr);
		vkFreeMemory(device.GetDevice(), m_perFrameDataBuffersMemory[i], nullptr);
	}


	vkDestroyDescriptorPool(device.GetDevice(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device.GetDevice(), m_descriptorSetLayout, nullptr);

	vkDestroyBuffer(device.GetDevice(), m_indexBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), m_indexBufferMemory, nullptr);
	vkDestroyBuffer(device.GetDevice(), m_vertexBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), m_vertexBufferMemory, nullptr);

	vkDestroyPipeline(device.GetDevice(), m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device.GetDevice(), m_pipelineLayout, nullptr);
	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		vkDestroySemaphore(device.GetDevice(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device.GetDevice(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device.GetDevice(), m_inFrameFences[i], nullptr);
	}
	vkDestroyCommandPool(device.GetDevice(), device.GetCommandPool(), nullptr);

	device.DestroyDevice();
}

void VlkRenderer::CreateShaderObject(CompiledShader& shader)
{
	CreateShaderModule(shader);
}

FrameBufferTexture* Hail::VlkRenderer::FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat)
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VlkFrameBufferTexture* frameBuffer = new VlkFrameBufferTexture(resolution, format, depthFormat);
	frameBuffer->SetName(name);
	frameBuffer->CreateFrameBufferTextureObjects(device);
	return frameBuffer;
}

void Hail::VlkRenderer::FrameBufferTexture_ClearFrameBuffer(FrameBufferTexture& frameBuffer)
{

}

void Hail::VlkRenderer::FrameBufferTexture_BindAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint)
{

}

void Hail::VlkRenderer::FrameBufferTexture_SetAsRenderTargetAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint)
{

}

void Hail::VlkRenderer::FrameBufferTexture_EndRenderAsTarget(FrameBufferTexture& frameBuffer)
{

}

void Hail::VlkRenderer::UpdateUniformBuffer(uint32_t frameInFlight)
{
	float deltaTime = m_timer->GetDeltaTime();
	float totalTime = m_timer->GetTotalTime();

	PerFrameUniformBuffer perFrameData{};

	perFrameData.totalTime_horizonLevel.x = totalTime;
	perFrameData.totalTime_horizonLevel.y = 0.0f;
	perFrameData.mainRenderResolution = m_renderTargetResolution;
	perFrameData.mainWindowResolution = m_windowResolution;
	memcpy(m_perFrameDataBuffersMapped[m_swapChain.GetCurrentFrame()], &perFrameData, sizeof(perFrameData));

	TutorialUniformBufferObject ubo{};

	ubo.model = glm::rotate(glm::mat4(1.0f), deltaTime * glm::radians(1.0f) + totalTime * 0.15f, glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = Transform3D::GetMatrix(g_camera.GetTransform());
	VkExtent2D swapExtent = m_swapChain.GetSwapChainExtent();
	ubo.proj = glm::perspective(glm::radians(g_camera.GetFov()), static_cast<float>(m_renderTargetResolution.x) / static_cast<float>(m_renderTargetResolution.y), g_camera.GetNear(), g_camera.GetFar());
	ubo.proj[1][1] *= -1;
	memcpy(m_uniformBuffersMapped[m_swapChain.GetCurrentFrame()], &ubo, sizeof(ubo));

	if(m_spriteInstanceData.Empty())

	memcpy(m_spriteBufferMapped[m_swapChain.GetCurrentFrame()], m_spriteInstanceData.Data(), sizeof(SpriteInstanceData) * m_spriteInstanceData.Size());

}

void VlkRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	const uint32_t frameInFlightIndex = m_swapChain.GetCurrentFrame();
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to begin recording command buffer!");
#endif
	}
	const glm::uvec2 mainPassResolution = m_mainPassFrameBufferTexture->GetResolution();
	VkExtent2D mainExtent = { mainPassResolution.x, mainPassResolution.y };
	VkRenderPassBeginInfo mainRenderPassInfo{};
	mainRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	mainRenderPassInfo.renderPass = m_mainRenderPass;
	mainRenderPassInfo.framebuffer = m_mainFrameBuffer[frameInFlightIndex];
	mainRenderPassInfo.renderArea.offset = { 0, 0 };
	mainRenderPassInfo.renderArea.extent = mainExtent;

	const glm::vec3 mainClearColor = m_mainPassFrameBufferTexture->GetClearColor();
	VkClearValue mainClearColors[2];
	mainClearColors[0].color = { mainClearColor.x, mainClearColor.y, mainClearColor.z, 1.0f };
	mainClearColors[1].depthStencil = { 1.0f, 0 };

	mainRenderPassInfo.clearValueCount = 2;
	mainRenderPassInfo.pClearValues = mainClearColors;

	vkCmdBeginRenderPass(commandBuffer, &mainRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport mainViewport{};
	mainViewport.x = 0.0f;
	mainViewport.y = 0.0f;
	mainViewport.width = static_cast<float>(mainPassResolution.x);
	mainViewport.height = static_cast<float>(mainPassResolution.y);
	mainViewport.minDepth = 0.0f;
	mainViewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &mainViewport);

	VkRect2D mainScissor{};
	mainScissor.offset = { 0, 0 };
	mainScissor.extent = mainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &mainScissor);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_mainPipeline);

	VkBuffer mainVertexBuffers[] = { m_vertexBuffer };
	VkDeviceSize mainOffsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, mainVertexBuffers, mainOffsets);
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_mainPipelineLayout, 0, 1, &m_mainPassDescriptorSet[frameInFlightIndex], 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_resourceManager->m_unitCube.indices.Size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	VkRenderPassBeginInfo spriteRenderPassInfo{};
	spriteRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	spriteRenderPassInfo.renderPass = m_spriteRenderPass;
	spriteRenderPassInfo.framebuffer = m_mainFrameBuffer[frameInFlightIndex];
	spriteRenderPassInfo.renderArea.offset = { 0, 0 };
	spriteRenderPassInfo.renderArea.extent = mainExtent;
	spriteRenderPassInfo.clearValueCount = 2;
	spriteRenderPassInfo.pClearValues = mainClearColors;
	vkCmdBeginRenderPass(commandBuffer, &spriteRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(commandBuffer, 0, 1, &mainViewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &mainScissor);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_spritePipeline);
	VkBuffer spriteVertexBuffer[] = { m_spriteVertexBuffer };
	VkDeviceSize spriteOffsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, spriteVertexBuffer, spriteOffsets);
	VkDescriptorSet descriptorSets[2] = { m_globalDescriptorSetsPerFrame[frameInFlightIndex], m_globalDescriptorSetsMaterial0[frameInFlightIndex]};
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_globalPipelineLayout, 0, 2, descriptorSets, 0, nullptr);
	glm::uvec4 pushConstants_instanceID_padding = { 0, 0, 0, 0 };

	const uint32_t numberOfSprites = m_spriteInstanceData.Size();
	for (size_t spriteInstance = 0; spriteInstance < numberOfSprites; spriteInstance++)
	{

		if (spriteInstance == 5)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_globalPipelineLayout, 1, 1, &m_globalDescriptorSetsMaterial0[frameInFlightIndex], 0, nullptr);
		}
		else
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_globalPipelineLayout, 1, 1, &m_globalDescriptorSetsMaterial1[frameInFlightIndex], 0, nullptr);
		}


		vkCmdPushConstants(commandBuffer, m_globalPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::uvec4), &pushConstants_instanceID_padding);
		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		pushConstants_instanceID_padding.x++;
	}
	vkCmdEndRenderPass(commandBuffer);


	VkExtent2D swapChainExtent = m_swapChain.GetSwapChainExtent();
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_swapChain.GetRenderPass();
	renderPassInfo.framebuffer = m_swapChain.GetFrameBuffer()[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;

	VkClearValue clearColors[2];
	clearColors[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearColors[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearColors;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	VkBuffer vertexBuffers[] = { m_fullscreenVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[m_swapChain.GetCurrentFrame()], 0, nullptr);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);


	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to record command buffer!");
#endif
	}
}


void VlkRenderer::CreateGraphicsPipeline()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkShaderModule vertShaderModule = nullptr;
	VkShaderModule fragShaderModule = nullptr;

	GrowingArray<CompiledShader>& requiredShaders = *m_resourceManager->GetShaderManager().GetRequiredShaders();
	//TODO: Make a nice system for loading the main shaders once the pipeline has been set up properly
	for (uint32_t i = 0; i < REQUIREDSHADERCOUNT; i++)
	{
		if (requiredShaders[i].shaderName == String64("VS_fullscreenPass"))
		{
			vertShaderModule = CreateShaderModule(requiredShaders[i]);
		} 
		else if (requiredShaders[i].shaderName == String64("FS_fullscreenPass"))
		{
			fragShaderModule = CreateShaderModule(requiredShaders[i]);
		}
	}
	if (!fragShaderModule || !vertShaderModule)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create shader module!");
#endif
		return;
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkDynamicState dynamicStates[2] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	const VkExtent2D swapChainExtent = m_swapChain.GetSwapChainExtent();
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	//rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
#ifdef DEBUG
		throw std::runtime_error("failed to create pipeline layout!");
#endif
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_swapChain.GetRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	VkVertexInputAttributeDescription attributeDescription{};
	attributeDescription.binding = 0;
	attributeDescription.location = 0;
	attributeDescription.format = VK_FORMAT_R32_UINT;
	attributeDescription.offset = 0;

	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(uint32_t);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(1);
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

	if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create graphics pipeline!");
#endif
	}

	vkDestroyShaderModule(device.GetDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(device.GetDevice(), vertShaderModule, nullptr);
}

void Hail::VlkRenderer::CreateMainGraphicsPipeline()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkShaderModule vertShaderModule = nullptr;
	VkShaderModule fragShaderModule = nullptr;

	GrowingArray<CompiledShader>& requiredShaders = *m_resourceManager->GetShaderManager().GetRequiredShaders();
	//TODO: Make a nice system for loading the main shaders once the pipeline has been set up properly
	for (uint32_t i = 0; i < REQUIREDSHADERCOUNT; i++)
	{
		if (requiredShaders[i].shaderName == String64("VS_triangle"))
		{
			vertShaderModule = CreateShaderModule(requiredShaders[i]);
		}
		else if (requiredShaders[i].shaderName == String64("FS_triangle"))
		{
			fragShaderModule = CreateShaderModule(requiredShaders[i]);
		}
	}
	if (!fragShaderModule || !vertShaderModule)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create shader module!");
#endif
		return;
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkDynamicState dynamicStates[2] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	const VkExtent2D passExtent = { m_renderTargetResolution.x, m_renderTargetResolution.y };
	viewport.width = (float)passExtent.width;
	viewport.height = (float)passExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = passExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	//rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &m_mainPassDescriptorSetLayout; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_mainPipelineLayout) != VK_SUCCESS) {
#ifdef DEBUG
		throw std::runtime_error("failed to create pipeline layout!");
#endif
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = m_mainPipelineLayout;
	pipelineInfo.renderPass = m_mainRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	auto bindingDescription = GetBindingDescription(VERTEX_TYPES::MODEL);
	auto attributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::MODEL);

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.Size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.Data();

	if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_mainPipeline) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create graphics pipeline!");
#endif
	}

	vkDestroyShaderModule(device.GetDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(device.GetDevice(), vertShaderModule, nullptr);
}

void Hail::VlkRenderer::CreateMainRenderPass()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = ToVkFormat(m_mainPassFrameBufferTexture->GetTextureFormat());
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = ToVkFormat(m_mainPassFrameBufferTexture->GetDepthFormat());
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device.GetDevice(), &renderPassInfo, nullptr, &m_mainRenderPass) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create render pass!");
#endif
	}
}

void Hail::VlkRenderer::CreateMainFrameBuffer()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	m_mainFrameBuffer[0] = VK_NULL_HANDLE;
	m_mainFrameBuffer[1] = VK_NULL_HANDLE;
	for (uint32_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{

		VlkFrameBufferTexture* mainFrameBufferTexture = reinterpret_cast<VlkFrameBufferTexture*>(m_mainPassFrameBufferTexture);
		FrameBufferTextureData colorTexture = mainFrameBufferTexture->GetTextureImage(i);
		FrameBufferTextureData depthTexture = mainFrameBufferTexture->GetDepthTextureImage(i);
		VkImageView attachments[2] = { colorTexture.imageView, depthTexture.imageView };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_mainRenderPass;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = mainFrameBufferTexture->GetResolution().x;
		framebufferInfo.height = mainFrameBufferTexture->GetResolution().y;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.GetDevice(), &framebufferInfo, nullptr, &m_mainFrameBuffer[i]) != VK_SUCCESS)
		{
#ifdef DEBUG
			throw std::runtime_error("failed to create framebuffer!");
#endif
		}
	}

}

void Hail::VlkRenderer::CreateSpriteGraphicsPipeline()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkShaderModule vertShaderModule = nullptr;
	VkShaderModule fragShaderModule = nullptr;

	GrowingArray<CompiledShader>& requiredShaders = *m_resourceManager->GetShaderManager().GetRequiredShaders();
	//TODO: Make a nice system for loading the main shaders once the pipeline has been set up properly
	for (uint32_t i = 0; i < REQUIREDSHADERCOUNT; i++)
	{
		if (requiredShaders[i].shaderName == String64("VS_Sprite"))
		{
			vertShaderModule = CreateShaderModule(requiredShaders[i]);
		}
		else if (requiredShaders[i].shaderName == String64("FS_Sprite"))
		{
			fragShaderModule = CreateShaderModule(requiredShaders[i]);
		}
	}
	if (!fragShaderModule || !vertShaderModule)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create sprite shader module!");
#endif
		return;
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkDynamicState dynamicStates[2] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	const VkExtent2D passExtent = { m_renderTargetResolution.x, m_renderTargetResolution.y };
	viewport.width = (float)passExtent.width;
	viewport.height = (float)passExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = passExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	//rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional


	//setup push constants
	VkPushConstantRange push_constant;
	//this push constant range starts at the beginning
	push_constant.offset = 0;
	//this push constant range takes up the size of a MeshPushConstants struct
	push_constant.size = sizeof(uint32_t) * 4;
	//this push constant range is accessible only in the vertex shader
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayout layouts[2] = { m_globalDescriptorSetLayoutPerFrame, m_globalDescriptorSetLayoutMaterial };
	VkPipelineLayoutCreateInfo globalPipelineLayoutInfo{};
	globalPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	globalPipelineLayoutInfo.setLayoutCount = 2; // Optional
	globalPipelineLayoutInfo.pSetLayouts = layouts; // Optional
	globalPipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
	globalPipelineLayoutInfo.pPushConstantRanges = &push_constant; // Optional

	if (vkCreatePipelineLayout(device.GetDevice(), &globalPipelineLayoutInfo, nullptr, &m_globalPipelineLayout) != VK_SUCCESS) {
#ifdef DEBUG
		throw std::runtime_error("failed to create pipeline layout!");
#endif
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = m_globalPipelineLayout;
	pipelineInfo.renderPass = m_spriteRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	auto bindingDescription = GetBindingDescription(VERTEX_TYPES::SPRITE);
	auto attributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::SPRITE);

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.Size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.Data();

	if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_spritePipeline) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create graphics pipeline!");
#endif
	}

	vkDestroyShaderModule(device.GetDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(device.GetDevice(), vertShaderModule, nullptr);
}

void Hail::VlkRenderer::CreateSpriteRenderPass()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = ToVkFormat(m_mainPassFrameBufferTexture->GetTextureFormat());
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = ToVkFormat(m_mainPassFrameBufferTexture->GetDepthFormat());
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device.GetDevice(), &renderPassInfo, nullptr, &m_spriteRenderPass) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create render pass!");
#endif
	}
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

VkShaderModule VlkRenderer::CreateShaderModule(CompiledShader& shader)
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shader.header.sizeOfShaderData;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.compiledCode);
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
#ifdef DEBUG
		throw std::runtime_error("failed to create shader module!");
#endif
		return nullptr;
	}
	shader.loadState = SHADER_LOADSTATE::UPLOADED_TO_GPU;
	SAFEDELETE_ARRAY(shader.compiledCode);
	return shaderModule;
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

void Hail::VlkRenderer::InitGlobalDescriptors()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	// SWIMMING POOOL 
	std::array<VkDescriptorPoolSize, 4> poolSizes =
	{
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
		}

	};

	VkDescriptorPoolCreateInfo finalPoolInfo{};
	finalPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	finalPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	finalPoolInfo.pPoolSizes = poolSizes.data();
	finalPoolInfo.maxSets = 10;
	finalPoolInfo.flags = 0;
	if (vkCreateDescriptorPool(device.GetDevice(), &finalPoolInfo, nullptr, &m_globalDescriptorPool) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create final descriptor pool!");
#endif
	}

	m_globalDescriptorSetLayoutPerFrame = CreateSetLayoutDescriptor({
		{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,  GetUniformBufferIndex(Hail::BUFFERS::PER_FRAME_DATA), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
		{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, GetUniformBufferIndex(Hail::BUFFERS::SPRITE_INSTANCE_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
		});

	m_globalDescriptorSetLayoutMaterial = CreateSetLayoutDescriptor({
		{ VK_SHADER_STAGE_FRAGMENT_BIT,  1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }
		});

	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		VkDescriptorSetAllocateInfo perMaterialAllocateInfo{};
		perMaterialAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perMaterialAllocateInfo.descriptorPool = m_globalDescriptorPool;
		perMaterialAllocateInfo.descriptorSetCount = 1;
		perMaterialAllocateInfo.pSetLayouts = &m_globalDescriptorSetLayoutMaterial;

		if (vkAllocateDescriptorSets(device.GetDevice(), &perMaterialAllocateInfo, &m_globalDescriptorSetsMaterial0[i]) != VK_SUCCESS)
		{
#ifdef DEBUG
			throw std::runtime_error("failed to allocate descriptor sets!");
#endif
		}
		if (vkAllocateDescriptorSets(device.GetDevice(), &perMaterialAllocateInfo, &m_globalDescriptorSetsMaterial1[i]) != VK_SUCCESS)
		{
#ifdef DEBUG
			throw std::runtime_error("failed to allocate descriptor sets!");
#endif
		}

		VkDescriptorSetAllocateInfo perFrameAllocateInfo{};
		perFrameAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perFrameAllocateInfo.descriptorPool = m_globalDescriptorPool;
		perFrameAllocateInfo.descriptorSetCount = 1;
		perFrameAllocateInfo.pSetLayouts = &m_globalDescriptorSetLayoutPerFrame;

		if (vkAllocateDescriptorSets(device.GetDevice(), &perFrameAllocateInfo, &m_globalDescriptorSetsPerFrame[i]) != VK_SUCCESS)
		{
#ifdef DEBUG
			throw std::runtime_error("failed to allocate descriptor sets!");
#endif
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_perFrameDataBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = GetUniformBufferSize(BUFFERS::PER_FRAME_DATA);

		VkDescriptorBufferInfo spriteBufferInfo{};
		spriteBufferInfo.buffer = m_spriteBuffer[i];
		spriteBufferInfo.offset = 0;
		spriteBufferInfo.range = GetUniformBufferSize(BUFFERS::SPRITE_INSTANCE_BUFFER);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_resourceManager->GetVulkanResources().GetTextureData(0).textureImageView;
		imageInfo.sampler = m_textureSampler;

		VkDescriptorImageInfo imageInfo1{};
		imageInfo1.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo1.imageView = m_resourceManager->GetVulkanResources().GetTextureData(1).textureImageView;
		imageInfo1.sampler = m_textureSampler;

		VkWriteDescriptorSet perFrameBuffer = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_globalDescriptorSetsPerFrame[i], &bufferInfo, 0);
		VkWriteDescriptorSet spriteInstanceBuffer = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_globalDescriptorSetsPerFrame[i], &spriteBufferInfo, 1);
		VkWriteDescriptorSet material0Write = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_globalDescriptorSetsMaterial0[i], &imageInfo1, 1);
		VkWriteDescriptorSet material1Write = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_globalDescriptorSetsMaterial1[i], &imageInfo, 1);


		VkWriteDescriptorSet setWrites[] = { perFrameBuffer,spriteInstanceBuffer, material0Write, material1Write };

		vkUpdateDescriptorSets(device.GetDevice(), 4, setWrites, 0, nullptr);

	}

}

void Hail::VlkRenderer::CreateSpriteDescriptorSets()
{

	//for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	//{
	//	std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

	//	VkDescriptorBufferInfo bufferInfo{};
	//	bufferInfo.buffer = m_uniformBuffers[i];
	//	bufferInfo.offset = 0;
	//	bufferInfo.range = GetUniformBufferSize(BUFFERS::TUTORIAL);

	//	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//	descriptorWrites[0].dstSet = m_globalDescriptorSets[i];
	//	descriptorWrites[0].dstBinding = GetUniformBufferIndex(Hail::BUFFERS::PER_FRAME_DATA);
	//	descriptorWrites[0].dstArrayElement = 0;
	//	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//	descriptorWrites[0].descriptorCount = 1;
	//	descriptorWrites[0].pBufferInfo = &bufferInfo;

	//	VkDescriptorBufferInfo spreiteBufferInfo{};
	//	spreiteBufferInfo.buffer = m_spriteBuffer[i];
	//	spreiteBufferInfo.offset = 0;
	//	spreiteBufferInfo.range = GetUniformBufferSize(BUFFERS::SPRITE_INSTANCE_BUFFER);

	//	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//	descriptorWrites[1].dstSet = m_globalDescriptorSets[i];
	//	descriptorWrites[1].dstBinding = GetUniformBufferIndex(Hail::BUFFERS::SPRITE_INSTANCE_BUFFER);
	//	descriptorWrites[1].dstArrayElement = 0;
	//	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	//	descriptorWrites[1].descriptorCount = 1;
	//	descriptorWrites[1].pBufferInfo = &spreiteBufferInfo;


	//	VkDescriptorImageInfo imageInfo{};
	//	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//	imageInfo.imageView = m_textureImageView;
	//	imageInfo.sampler = m_textureSampler;

	//	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//	descriptorWrites[2].dstSet = m_globalDescriptorSets[i];
	//	descriptorWrites[2].dstBinding = 1;
	//	descriptorWrites[2].dstArrayElement = 0;
	//	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//	descriptorWrites[2].descriptorCount = 1;
	//	descriptorWrites[2].pImageInfo = &imageInfo;

	//	vkUpdateDescriptorSets(device.GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	//}
}


void Hail::VlkRenderer::CreateUniformBuffers()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkDeviceSize bufferSize = GetUniformBufferSize(BUFFERS::TUTORIAL);

	m_uniformBuffers.InitAndFill(MAX_FRAMESINFLIGHT);
	m_uniformBuffersMemory.InitAndFill(MAX_FRAMESINFLIGHT);
	m_uniformBuffersMapped.InitAndFill(MAX_FRAMESINFLIGHT);

	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++) 
	{
		CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
		vkMapMemory(device.GetDevice(), m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
	}

	m_spriteBuffer.InitAndFill(MAX_FRAMESINFLIGHT);
	m_spriteBufferMemory.InitAndFill(MAX_FRAMESINFLIGHT);
	m_spriteBufferMapped.InitAndFill(MAX_FRAMESINFLIGHT);

	m_perFrameDataBuffers.InitAndFill(MAX_FRAMESINFLIGHT);
	m_perFrameDataBuffersMemory.InitAndFill(MAX_FRAMESINFLIGHT);
	m_perFrameDataBuffersMapped.InitAndFill(MAX_FRAMESINFLIGHT);

	const VkDeviceSize spriteBufferSize = GetUniformBufferSize(BUFFERS::SPRITE_INSTANCE_BUFFER);
	const VkDeviceSize perFrameBufferSize = GetUniformBufferSize(BUFFERS::PER_FRAME_DATA);
	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		CreateBuffer(device, spriteBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_spriteBuffer[i], m_spriteBufferMemory[i]);
		vkMapMemory(device.GetDevice(), m_spriteBufferMemory[i], 0, spriteBufferSize, 0, &m_spriteBufferMapped[i]);

		CreateBuffer(device, perFrameBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_perFrameDataBuffers[i], m_perFrameDataBuffersMemory[i]);
		vkMapMemory(device.GetDevice(), m_perFrameDataBuffersMemory[i], 0, perFrameBufferSize, 0, &m_perFrameDataBuffersMapped[i]);
	}



}

void Hail::VlkRenderer::CreateDescriptorPool()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	std::array <VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = MAX_FRAMESINFLIGHT;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_FRAMESINFLIGHT;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = MAX_FRAMESINFLIGHT;
	poolInfo.flags = 0;
	if (vkCreateDescriptorPool(device.GetDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create descriptor pool!");
#endif
	}
}

void Hail::VlkRenderer::CreateDescriptorSets()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	GrowingArray<VkDescriptorSetLayout> layouts(MAX_FRAMESINFLIGHT, m_descriptorSetLayout, false);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMESINFLIGHT);
	allocInfo.pSetLayouts = layouts.Data(); 
	
	m_descriptorSets.InitAndFill(MAX_FRAMESINFLIGHT);
	if (vkAllocateDescriptorSets(device.GetDevice(), &allocInfo, m_descriptorSets.Data()) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to allocate descriptor sets!");
#endif
	}

	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++) 
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_perFrameDataBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = GetUniformBufferSize(BUFFERS::PER_FRAME_DATA);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = reinterpret_cast<VlkFrameBufferTexture*>(m_mainPassFrameBufferTexture)->GetTextureImage(i).imageView;
		imageInfo.sampler = m_pointTextureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device.GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Hail::VlkRenderer::CreateDescriptorSetLayout()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	GrowingArray<VkDescriptorSetLayoutBinding>bindings = { uboLayoutBinding, samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.Size();
	layoutInfo.pBindings = bindings.Data();

	if (vkCreateDescriptorSetLayout(device.GetDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create final pass descriptor set layout!");
#endif
	}

}

VkDescriptorSetLayout Hail::VlkRenderer::CreateSetLayoutDescriptor(GrowingArray<VlkLayoutDescriptor> descriptors)
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkDescriptorSetLayout returnDescriptor = VK_NULL_HANDLE;
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
	if (vkCreateDescriptorSetLayout(device.GetDevice(), &layoutSetInfo, nullptr, &returnDescriptor) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create final pass descriptor set layout!");
#endif
	}
	return returnDescriptor;
}

VkWriteDescriptorSet Hail::VlkRenderer::WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.dstBinding = binding;
	write.dstSet = dstSet;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = bufferInfo;
	return write;
}

VkWriteDescriptorSet Hail::VlkRenderer::WriteDescriptorSampler(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* bufferInfo, uint32_t binding)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.dstBinding = binding;
	write.dstSet = dstSet;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = bufferInfo;
	return write;
}

void Hail::VlkRenderer::CreateMainDescriptorPool()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	std::array <VkDescriptorPoolSize, 2> finalPoolSizes{};
	finalPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	finalPoolSizes[0].descriptorCount = MAX_FRAMESINFLIGHT;
	finalPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	finalPoolSizes[1].descriptorCount = MAX_FRAMESINFLIGHT;

	VkDescriptorPoolCreateInfo finalPoolInfo{};
	finalPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	finalPoolInfo.poolSizeCount = static_cast<uint32_t>(finalPoolSizes.size());
	finalPoolInfo.pPoolSizes = finalPoolSizes.data();
	finalPoolInfo.maxSets = MAX_FRAMESINFLIGHT;
	finalPoolInfo.flags = 0;
	if (vkCreateDescriptorPool(device.GetDevice(), &finalPoolInfo, nullptr, &m_mainPassDescriptorPool) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create final descriptor pool!");
#endif
	}
}

void Hail::VlkRenderer::CreateMainDescriptorSets()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	GrowingArray<VkDescriptorSetLayout> layouts(MAX_FRAMESINFLIGHT, m_mainPassDescriptorSetLayout, false);
	VkDescriptorSetAllocateInfo finalPassAllocInfo{};
	finalPassAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	finalPassAllocInfo.descriptorPool = m_mainPassDescriptorPool;
	finalPassAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMESINFLIGHT);
	finalPassAllocInfo.pSetLayouts = layouts.Data();

	m_mainPassDescriptorSet.InitAndFill(MAX_FRAMESINFLIGHT);

	if (vkAllocateDescriptorSets(device.GetDevice(), &finalPassAllocInfo, m_mainPassDescriptorSet.Data()) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to allocate descriptor sets!");
#endif
	}
	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = GetUniformBufferSize(BUFFERS::TUTORIAL);

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_mainPassDescriptorSet[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_resourceManager->GetVulkanResources().GetTextureData(0).textureImageView;
		imageInfo.sampler = m_textureSampler;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_mainPassDescriptorSet[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device.GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Hail::VlkRenderer::CreateMainDescriptorSetLayout()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	GrowingArray<VkDescriptorSetLayoutBinding>bindings = { uboLayoutBinding, samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.Size();
	layoutInfo.pBindings = bindings.Data();

	if (vkCreateDescriptorSetLayout(device.GetDevice(), &layoutInfo, nullptr, &m_mainPassDescriptorSetLayout) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create final pass descriptor set layout!");
#endif
	}
}




