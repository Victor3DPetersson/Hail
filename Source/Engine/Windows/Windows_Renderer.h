//Interface for the entire engine
#pragma once

#include "Renderer.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "Containers\GrowingArray\GrowingArray.h"


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
	bool Init(RESOLUTIONS startupResolution, ShaderManager* shaderManager) override;
	void MainLoop() override;
	void Cleanup() override;
	void InitImGui();

	void CreateShaderObject(CompiledShader& shader) override;

private:

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
	void CreateImageViews();
	
	void CreateLogicalDevice();
	void CreateWindowsSurface();

	void CreateRenderPass();
	void CreatGraphicsPipeline();

	void CreateFramebuffers();

	void CreateCommandPool();
	void CreateCommandBuffer();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void CreateSyncObjects();

	VkShaderModule CreateShaderModule(CompiledShader& shader);


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

	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

	VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

	GrowingArray<VkFramebuffer> m_swapChainFramebuffers;

	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;

	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;
	VkFence m_inFrameFence;

#ifdef DEBUG
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
#endif
};