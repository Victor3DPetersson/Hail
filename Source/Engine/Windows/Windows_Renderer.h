//Interface for the entire engine
#pragma once

#include "Renderer.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{
	const uint32_t MAX_FRAMESINFLIGHT = 2;

	struct QueueFamilyIndices {
		uint32_t graphicsFamily = INVALID_UINT;
		uint32_t presentFamily = INVALID_UINT;

		bool IsComplete() {
			return graphicsFamily != INVALID_UINT && presentFamily != INVALID_UINT;
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		GrowingArray<VkSurfaceFormatKHR> formats;
		GrowingArray<VkPresentModeKHR> presentModes;
	};

	class VlkRenderer : public Renderer
	{
	public:
		bool Init(RESOLUTIONS startupResolution, ShaderManager* shaderManager, TextureManager* textureManager, ResourceManager* resourceManager, Timer* timer) override;
		void StartFrame() final;
		void Render() final;
		void EndFrame() final;
		void Cleanup() final;
		void InitImGui();

		void RecreateSwapchain();
		void CreateShaderObject(CompiledShader& shader) final;

	protected:


	private:

		void CleanupSwapchain();

		void UpdateUniformBuffer(uint32_t frameInFlight);

		bool CreateInstance();
		bool CheckValidationLayerSupport();
		bool CheckRequiredExtensions();
		void SetupDebugMessenger();

		void PickPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

		void CreateSwapChain();
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const GrowingArray<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const GrowingArray<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkFormat FindSupportedFormat(const GrowingArray<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);

		void CreateImageViews();

		void CreateLogicalDevice();
		void CreateWindowsSurface();

		void CreateRenderPass();
		void CreatGraphicsPipeline();

		void CreateFramebuffers();

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
		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView  CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		void CreateDepthResources();

		void CreateDescriptorSetLayout();

		VkInstance m_vkInstance = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		VkQueue m_presentQueue = VK_NULL_HANDLE;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;

		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		GrowingArray<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		GrowingArray<VkImageView> m_swapChainImageViews;

		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;

		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		VkDescriptorSetLayout  m_descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

		VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

		GrowingArray<VkFramebuffer> m_swapChainFramebuffers;

		GrowingArray<VkBuffer> m_uniformBuffers;
		GrowingArray<VkDeviceMemory> m_uniformBuffersMemory;
		GrowingArray<void*> m_uniformBuffersMapped;

		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		GrowingArray<VkDescriptorSet> m_descriptorSets;

		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_commandBuffers[MAX_FRAMESINFLIGHT];

		VkSemaphore m_imageAvailableSemaphores[MAX_FRAMESINFLIGHT];
		VkSemaphore m_renderFinishedSemaphores[MAX_FRAMESINFLIGHT];
		VkFence m_inFrameFences[MAX_FRAMESINFLIGHT];

		uint32_t m_currentFrame = 0;
		uint32_t m_currentImageIndex = 0;

		VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer m_indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

		VkImage m_textureImage = VK_NULL_HANDLE;
		VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
		VkImageView m_textureImageView = VK_NULL_HANDLE;
		VkSampler m_textureSampler = VK_NULL_HANDLE;

		VkDescriptorPool m_imguiPool = VK_NULL_HANDLE;

#ifdef DEBUG
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
#endif
	};
}

