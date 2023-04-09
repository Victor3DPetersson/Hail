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
		bool InitDevice(RESOLUTIONS startupResolution, Timer* timer) override;
		bool InitGraphicsEngine(ResourceManager* resourceManager) override;
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

		void CreateSpriteGraphicsPipeline();
		void CreateSpriteRenderPass();
		void CreateSpriteDescriptorSets();

		void CreateCommandBuffers();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void CreateSyncObjects();

		VkShaderModule CreateShaderModule(CompiledShader& shader);

		void CreateVertexBuffer();
		void CreateFullscreenVertexBuffer();
		void CreateSpriteVertexBuffer();
		void CreateIndexBuffer();

		void InitGlobalDescriptors();

		//General functions

		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateDescriptorSetLayout();


		struct VlkLayoutDescriptor
		{
			VkShaderStageFlags flags;
			uint32_t bindingPoint;
			VkDescriptorType type;
		};
		VkDescriptorSetLayout CreateSetLayoutDescriptor(GrowingArray<VlkLayoutDescriptor> descriptors);
		VkWriteDescriptorSet WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
		VkWriteDescriptorSet WriteDescriptorSampler(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* bufferInfo, uint32_t binding);

		VlkSwapChain m_swapChain;



		VkDescriptorPool m_globalDescriptorPool = VK_NULL_HANDLE;
		VkPipelineLayout m_globalPipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout  m_globalDescriptorSetLayoutPerFrame = VK_NULL_HANDLE;
		VkDescriptorSetLayout  m_globalDescriptorSetLayoutMaterial = VK_NULL_HANDLE;
		VkDescriptorSet m_globalDescriptorSetsPerFrame[MAX_FRAMESINFLIGHT];
		VkDescriptorSet m_globalDescriptorSetsMaterial0[MAX_FRAMESINFLIGHT];
		VkDescriptorSet m_globalDescriptorSetsMaterial1[MAX_FRAMESINFLIGHT];


		VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

		VkCommandBuffer m_commandBuffers[MAX_FRAMESINFLIGHT];

		GrowingArray<VkBuffer> m_uniformBuffers;
		GrowingArray<VkDeviceMemory> m_uniformBuffersMemory;
		GrowingArray<void*> m_uniformBuffersMapped;

		VkDescriptorSetLayout  m_descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		GrowingArray<VkDescriptorSet> m_descriptorSets;


		////MODEL PASS
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
		////////////////

		///////////SPRITES

		VkRenderPass m_spriteRenderPass = VK_NULL_HANDLE;
		VkPipeline m_spritePipeline = VK_NULL_HANDLE;
		VkFramebuffer m_spriteFrameBuffer[MAX_FRAMESINFLIGHT];

		GrowingArray<VkBuffer> m_spriteBuffer;
		GrowingArray<VkDeviceMemory> m_spriteBufferMemory;
		GrowingArray<void*> m_spriteBufferMapped;

		///////////


		GrowingArray<VkBuffer> m_perFrameDataBuffers;
		GrowingArray<VkDeviceMemory> m_perFrameDataBuffersMemory;
		GrowingArray<void*> m_perFrameDataBuffersMapped;


		VkBuffer m_fullscreenVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_fullscreenVertexBufferMemory = VK_NULL_HANDLE;

		VkBuffer m_spriteVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_spriteVertexBufferMemory = VK_NULL_HANDLE;

		//Vertex and index buffer for cube
		VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer m_indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;


		VkSampler m_textureSampler = VK_NULL_HANDLE;
		VkSampler m_pointTextureSampler = VK_NULL_HANDLE;




		VkDescriptorPool m_imguiPool = VK_NULL_HANDLE;

		VkSemaphore m_imageAvailableSemaphores[MAX_FRAMESINFLIGHT];
		VkSemaphore m_renderFinishedSemaphores[MAX_FRAMESINFLIGHT];
		VkFence m_inFrameFences[MAX_FRAMESINFLIGHT];
	};
}

