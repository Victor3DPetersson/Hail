//Interface for the entire engine
#pragma once

#include "Renderer.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "VulkanInternal\VlkDevice.h"
#include "VulkanInternal\VlkSwapChain.h"

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
		void InitImGui();

		void CreateShaderObject(CompiledShader& shader) final;

	protected:


	private:

		void UpdateUniformBuffer(uint32_t frameInFlight);


		void CreatGraphicsPipeline();

		void CreateCommandPool();
		void CreateCommandBuffers();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void CreateSyncObjects();

		VkShaderModule CreateShaderModule(CompiledShader& shader);

		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();

		void CreateVertexBuffer();
		void CreateIndexBuffer();
		
		//General functions
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		void CreateDescriptorSetLayout();

		VlkDevice m_device;
		VlkSwapChain m_swapChain;

		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		VkQueue m_presentQueue = VK_NULL_HANDLE;

		VkDescriptorSetLayout  m_descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

		VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;


		GrowingArray<VkBuffer> m_uniformBuffers;
		GrowingArray<VkDeviceMemory> m_uniformBuffersMemory;
		GrowingArray<void*> m_uniformBuffersMapped;

		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		GrowingArray<VkDescriptorSet> m_descriptorSets;

		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_commandBuffers[MAX_FRAMESINFLIGHT];


		VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer m_indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

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

