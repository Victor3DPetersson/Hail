//Interface for the entire engine
#pragma once

#include "Renderer.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{

	class VlkDevice;
	class VlkSwapChain
	{
	public:
		void Init(VlkDevice& device);
		bool FrameStart(VlkDevice& device, VkFence* inFrameFences, VkSemaphore* imageAvailableSemaphores, bool resizeSwapChain);
		void FrameEnd(VkSemaphore* endSemaphore, VkQueue presentQueue);
		void DestroySwapChain(VlkDevice& device);

		VkExtent2D GetSwapChainExtent() { return m_swapChainExtent; }
		VkFormat GetSwapChainFormat() { return m_swapChainImageFormat; }
		uint32_t GetCurrentFrame() { return m_currentFrame; }
		uint32_t GetCurrentSwapImageIndex() { return m_currentImageIndex; }
		VkRenderPass GetRenderPass() { return m_finalRenderPass; }
		VkFramebuffer* GetFrameBuffer() { return m_swapChainFramebuffers.Data(); }
		VkFormat FindSupportedFormat(VlkDevice& device, const GrowingArray<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat(VlkDevice& device);

		//TEMP will remove later
		VkImage GetDepthImage() { return m_depthImage; }
		VkImageView GetDepthImageView() { return m_depthImageView; }

	private:
		void CreateSwapChain(VlkDevice& device);
		void RecreateSwapchain(VlkDevice& device, VkFence* inFrameFences, VkSemaphore* imageAvailableSemaphores);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const GrowingArray<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const GrowingArray<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void CleanupSwapchain(VlkDevice& device);
		void CreateImageViews(VlkDevice& device);
		void CreateFramebuffers(VlkDevice& device);
		void CreateRenderPass(VlkDevice& device);

		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		GrowingArray<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		GrowingArray<VkImageView> m_swapChainImageViews;

		GrowingArray<VkFramebuffer> m_swapChainFramebuffers;

		uint32_t m_currentImageIndex = 0;
		uint32_t m_currentFrame = 0;

		VkRenderPass m_finalRenderPass = VK_NULL_HANDLE;

		void CreateDepthResources(VlkDevice& device);
		//Move away from swapchain and render to a new target later
		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;
	};
}

