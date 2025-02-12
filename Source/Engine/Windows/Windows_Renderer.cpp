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
	Renderer::Initialize();

	//Create initial buffers
	m_swapChain = (VlkSwapChain*)m_pResourceManager->GetSwapChain();
	m_pContext->StartTransferPass();
	CreateFullscreenVertexBuffer();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateDebugLineVertexBuffer();
	CreateSpriteVertexBuffer();
	InitImGui();
	m_pContext->EndTransferPass();
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
	////execute a gpu command to upload imgui font textures
	//immediate_submit([&](VkCommandBuffer cmd) {
	//	ImGui_ImplVulkan_CreateFontsTexture(cmd);
	//	});

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
	Renderer::StartFrame(renderPool);
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplVulkan_NewFrame();

	ImGui::NewFrame();
	//g_camera = renderPool.renderCamera;
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

void Hail::VlkRenderer::RenderSprite(const RenderCommand_Sprite& spriteCommandToRender, uint32_t spriteInstance)
{
	const MaterialInstance& materialInstance = m_pResourceManager->GetMaterialManager()->GetMaterialInstance(m_commandPoolToRender->spriteCommands[spriteInstance].materialInstanceID, eMaterialType::SPRITE);
	//BindMaterialPipeline(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::SPRITE, materialInstance.m_materialIndex)->m_pPipeline, false);

	const uint32_t frameInFlightIndex = m_swapChain->GetFrameInFlight();
	VlkCommandBuffer* pVlkCommandBfr = (VlkCommandBuffer*)m_pContext->GetCurrentCommandBuffer();
	VkCommandBuffer commandBuffer = pVlkCommandBfr->m_commandBuffer;

	VlkMaterial& vlkMaterial = *(VlkMaterial*)m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::SPRITE, materialInstance.m_materialIndex);
	VlkPipeline& vlkPipeline = *(VlkPipeline*)vlkMaterial.m_pPipeline;


	// TODO move over vertrex buffers to the context or somewhere else
	VkDeviceSize spriteOffsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_pSpriteVertexBuffer->GetBuffer(0), spriteOffsets);

	// TODO: use the reflected push constants
	glm::uvec4 pushConstants_instanceID_padding = { spriteInstance, 0, 0, 0 };
	const MaterialInstance& instanceMaterialData = m_pResourceManager->GetMaterialManager()->GetMaterialInstance(spriteCommandToRender.materialInstanceID, eMaterialType::SPRITE);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vlkPipeline.m_pipelineLayout, 2, 1, &vlkMaterial.m_instanceDescriptors[instanceMaterialData.m_gpuResourceInstance].descriptors[frameInFlightIndex], 0, nullptr);
	vkCmdPushConstants(commandBuffer, vlkPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::uvec4), &pushConstants_instanceID_padding);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

void Hail::VlkRenderer::RenderMesh(const RenderCommand_Mesh& meshCommandToRender, uint32_t meshInstance)
{
	const uint32_t frameInFlightIndex = m_swapChain->GetFrameInFlight();
	VlkCommandBuffer* pVlkCommandBfr = (VlkCommandBuffer*)m_pContext->GetCurrentCommandBuffer();
	VkCommandBuffer commandBuffer = pVlkCommandBfr->m_commandBuffer;

	VlkMaterial& vlkMaterial = *(VlkMaterial*)m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::MODEL3D, 0);
	VlkPipeline& vlkPipeline = *(VlkPipeline*)vlkMaterial.m_pPipeline;

	// TODO: move to the context a bind function for material instance data
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vlkPipeline.m_pipelineLayout, 2, 1, &vlkMaterial.m_instanceDescriptors[0].descriptors[frameInFlightIndex], 0, nullptr);

	//Todo: Make sure to fix vertex binding and that kind of stuff later and make the resources be held outside of renderer
	VkBuffer mainVertexBuffers[] = { m_pVertexBuffer->GetBuffer(0) };
	VkDeviceSize mainOffsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, mainVertexBuffers, mainOffsets);
	vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffer->GetBuffer(0), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_pResourceManager->m_unitCube.indices.Size()), 1, 0, 0, 0);
}

void Hail::VlkRenderer::RenderDebugLines2D(uint32 numberOfLinesToRender, uint32 offsetFrom3DLines)
{
	const uint32_t frameInFlightIndex = m_swapChain->GetFrameInFlight();
	VlkCommandBuffer* pVlkCommandBfr = (VlkCommandBuffer*)m_pContext->GetCurrentCommandBuffer();
	VkCommandBuffer commandBuffer = pVlkCommandBfr->m_commandBuffer;

	VkBuffer mainVertexBuffers[] = { m_pDebugLineVertexBuffer->GetBuffer(0)};
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
	VlkCommandBuffer* pVlkCommandBuffer = (VlkCommandBuffer*)m_pContext->GetCurrentCommandBuffer();
	VkCommandBuffer& commandBuffer = pVlkCommandBuffer->m_commandBuffer;

	VkBuffer vertexBuffers[] = { m_pFullscreenVertexBuffer->GetBuffer(0) };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

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

	m_pFullscreenVertexBuffer->CleanupResource(m_renderDevice);
	m_pSpriteVertexBuffer->CleanupResource(m_renderDevice);
	m_pVertexBuffer->CleanupResource(m_renderDevice);
	m_pIndexBuffer->CleanupResource(m_renderDevice);
	m_pDebugLineVertexBuffer->CleanupResource(m_renderDevice);
	SAFEDELETE(m_pFullscreenVertexBuffer);
	SAFEDELETE(m_pSpriteVertexBuffer);
	SAFEDELETE(m_pVertexBuffer);
	SAFEDELETE(m_pIndexBuffer);
	SAFEDELETE(m_pDebugLineVertexBuffer);
	SAFEDELETE(m_pContext);

	vkDestroyCommandPool(device.GetDevice(), device.GetCommandPool(), nullptr);

	device.DestroyDevice();
}

void VlkRenderer::CreateVertexBuffer()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);

	BufferProperties vertexBufferProperties;
	vertexBufferProperties.elementByteSize = sizeof(VertexModel);
	vertexBufferProperties.numberOfElements = m_pResourceManager->m_unitCube.vertices.Size();
	vertexBufferProperties.offset = 0;
	vertexBufferProperties.type = eBufferType::vertex;
	vertexBufferProperties.domain = eShaderBufferDomain::GpuOnly;
	vertexBufferProperties.usage = eShaderBufferUsage::Read;
	vertexBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
	m_pVertexBuffer = (VlkBufferObject*)m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(vertexBufferProperties);

	m_pContext->UploadDataToBuffer(m_pVertexBuffer, m_pResourceManager->m_unitCube.vertices.Data(), sizeof(VertexModel) * m_pResourceManager->m_unitCube.vertices.Size());
}

void Hail::VlkRenderer::CreateFullscreenVertexBuffer()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);

	uint32_t vertices[3] = { 0, 1, 2 };

	BufferProperties fullscreenVertexBufferProperties;
	fullscreenVertexBufferProperties.elementByteSize = sizeof(uint32);
	fullscreenVertexBufferProperties.numberOfElements = 3;
	fullscreenVertexBufferProperties.offset = 0;
	fullscreenVertexBufferProperties.type = eBufferType::vertex;
	fullscreenVertexBufferProperties.domain = eShaderBufferDomain::GpuOnly;
	fullscreenVertexBufferProperties.usage = eShaderBufferUsage::Read;
	fullscreenVertexBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
	m_pFullscreenVertexBuffer = (VlkBufferObject*)m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(fullscreenVertexBufferProperties);

	m_pContext->UploadDataToBuffer(m_pFullscreenVertexBuffer, vertices, 3 * sizeof(uint32));
}

void Hail::VlkRenderer::CreateSpriteVertexBuffer()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);

	uint32_t vertices[6] = { 0, 1, 2, 3, 4, 5 };

	BufferProperties spriteVertexBufferProperties;
	spriteVertexBufferProperties.elementByteSize = sizeof(uint32_t);
	spriteVertexBufferProperties.numberOfElements = 6;
	spriteVertexBufferProperties.offset = 0;
	spriteVertexBufferProperties.type = eBufferType::vertex;
	spriteVertexBufferProperties.domain = eShaderBufferDomain::GpuOnly;
	spriteVertexBufferProperties.usage = eShaderBufferUsage::Read;
	spriteVertexBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
	m_pSpriteVertexBuffer = (VlkBufferObject*)m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(spriteVertexBufferProperties);

	m_pContext->UploadDataToBuffer(m_pSpriteVertexBuffer, vertices, sizeof(uint32_t) * 6);
}

void VlkRenderer::CreateIndexBuffer()
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_renderDevice);

	BufferProperties indexBufferProperties;
	indexBufferProperties.elementByteSize = sizeof(uint32_t);
	indexBufferProperties.numberOfElements = m_pResourceManager->m_unitCube.indices.Size();
	indexBufferProperties.offset = 0;
	indexBufferProperties.type = eBufferType::index;
	indexBufferProperties.domain = eShaderBufferDomain::GpuOnly;
	indexBufferProperties.usage = eShaderBufferUsage::Read;
	indexBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
	m_pIndexBuffer = (VlkBufferObject*)m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(indexBufferProperties);
	m_pContext->UploadDataToBuffer(m_pIndexBuffer, m_pResourceManager->m_unitCube.indices.Data(), sizeof(uint32_t) * m_pResourceManager->m_unitCube.indices.Size());
}

void Hail::VlkRenderer::CreateDebugLineVertexBuffer()
{
	GrowingArray<uint32> debugLineVertices(MAX_NUMBER_OF_DEBUG_LINES, 0);
	for (uint32 i = 0; i < MAX_NUMBER_OF_DEBUG_LINES; i++)
	{
		debugLineVertices[i] = i;
	}

	BufferProperties debugLineBufferProperties;
	debugLineBufferProperties.elementByteSize = sizeof(uint32_t);
	debugLineBufferProperties.numberOfElements = MAX_NUMBER_OF_DEBUG_LINES;
	debugLineBufferProperties.offset = 0;
	debugLineBufferProperties.type = eBufferType::vertex;
	debugLineBufferProperties.domain = eShaderBufferDomain::GpuOnly;
	debugLineBufferProperties.usage = eShaderBufferUsage::Read;
	debugLineBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
	m_pDebugLineVertexBuffer = (VlkBufferObject*)m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(debugLineBufferProperties);
	m_pContext->UploadDataToBuffer(m_pDebugLineVertexBuffer, debugLineVertices.Data(), sizeof(uint32_t) * MAX_NUMBER_OF_DEBUG_LINES);
}
