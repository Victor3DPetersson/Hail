//Interface for the entire engine
#pragma once

#include "Renderer.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "VulkanInternal\VlkDevice.h"
#include "VulkanInternal\VlkSwapChain.h"
#include "VulkanInternal\VlkTextureCreationFunctions.h"

namespace Hail
{
	class VlkRenderer : public Renderer
	{
	public:
		bool Init(RESOLUTIONS startupResolution, ShaderManager* shaderManager, TextureManager* textureManager, ResourceManager* resourceManager, Timer* timer) override;
		void StartFrame(RenderCommandPool& renderPool) final;
		void Render() final;
		void EndFrame() final;
		void Cleanup() final;
		void InitImGui() final;

		void CreateShaderObject(CompiledShader& shader) final;

	protected:
		FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) final;
		void FrameBufferTexture_ClearFrameBuffer(FrameBufferTexture& frameBuffer) final;
		void FrameBufferTexture_BindAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) final;
		void FrameBufferTexture_SetAsRenderTargetAtSlot(FrameBufferTexture& frameBuffer, uint32_t bindingPoint) final;
		void FrameBufferTexture_EndRenderAsTarget(FrameBufferTexture& frameBuffer) final;

	private:

		void UpdateUniformBuffer(uint32_t frameInFlight);


		void CreateGraphicsPipeline();
		void CreateMainGraphicsPipeline();
		void CreateMainRenderPass();
		void CreateMainFrameBuffer();
		void CreateMainDescriptorPool();
		void CreateMainDescriptorSets();
		void CreateMainDescriptorSetLayout();


		void CreateCommandPool();
		void CreateCommandBuffers();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void CreateSyncObjects();

		VkShaderModule CreateShaderModule(CompiledShader& shader);

		void CreateTextureImage();
		void CreateTextureImageView();

		void CreateVertexBuffer();
		void CreateFullscreenVertexBuffer();
		void CreateIndexBuffer();
		
		//General functions

		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateDescriptorSetLayout();




		VlkDevice m_device;
		VlkSwapChain m_swapChain;

		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		VkQueue m_presentQueue = VK_NULL_HANDLE;
		VkQueue m_computeQueue = VK_NULL_HANDLE;


		VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

		GrowingArray<VkBuffer> m_uniformBuffers;
		GrowingArray<VkDeviceMemory> m_uniformBuffersMemory;
		GrowingArray<void*> m_uniformBuffersMapped;

		VkDescriptorSetLayout  m_descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		GrowingArray<VkDescriptorSet> m_descriptorSets;

		//Descriptor data for finalRenderpass
		VkDescriptorPool m_mainPassDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout  m_mainPassDescriptorSetLayout = VK_NULL_HANDLE;
		//Has To be recreated when resized frame
		GrowingArray <VkDescriptorSet> m_mainPassDescriptorSet;

		//ModelPipeline
		VkRenderPass m_mainRenderPass = VK_NULL_HANDLE;
		VkPipeline m_mainPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_mainPipelineLayout = VK_NULL_HANDLE;
		VkFramebuffer m_mainFrameBuffer[MAX_FRAMESINFLIGHT];

		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_commandBuffers[MAX_FRAMESINFLIGHT];


		VkBuffer m_perFrameDataBuffers;
		VkDeviceMemory m_perFrameDataBuffersMemory;
		void* m_perFrameDataBuffersMapped;


		VkBuffer m_fullscreenVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_fullscreenVertexBufferMemory = VK_NULL_HANDLE;

		//Vertex and index buffer for cube
		VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer m_indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

		//Texture data for the cube
		VkImage m_textureImage = VK_NULL_HANDLE;
		VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
		VkImageView m_textureImageView = VK_NULL_HANDLE;
		VkSampler m_textureSampler = VK_NULL_HANDLE;

		VkDescriptorPool m_imguiPool = VK_NULL_HANDLE;

		VkSemaphore m_imageAvailableSemaphores[MAX_FRAMESINFLIGHT];
		VkSemaphore m_renderFinishedSemaphores[MAX_FRAMESINFLIGHT];
		VkFence m_inFrameFences[MAX_FRAMESINFLIGHT];


	};
}

