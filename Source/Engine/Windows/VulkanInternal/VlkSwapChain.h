//Interface for the entire engine
#pragma once
#include "Rendering\SwapChain.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "VlkFrameBufferTexture.h"

namespace Hail
{

	class VlkDevice;
	class VlkSwapChain : public SwapChain
	{
	public:
		VlkSwapChain();
		void Init(RenderingDevice* renderDevice) override;
		bool FrameStart(VlkDevice& device, VkFence* inFrameFences, VkSemaphore* imageAvailableSemaphores);
		void FrameEnd(VkSemaphore* endSemaphore, VkQueue presentQueue);
		void DestroySwapChain(RenderingDevice* device) override;
		FrameBufferTexture* GetFrameBufferTexture() override;

		VkExtent2D GetSwapChainExtent() { return m_swapChainExtent; }
		VkFormat GetSwapChainFormat() { return m_swapChainImageFormat; }
		uint32_t GetCurrentFrame() { return m_currentFrame; }
		uint32_t GetCurrentSwapImageIndex() { return m_currentImageIndex; }
		uint32_t GetSwapchainImageCount(){ return m_imageCount; }
		VkRenderPass GetRenderPass() { return m_finalRenderPass; }
		VkFramebuffer GetFrameBuffer(uint32_t index);
		VkFormat FindSupportedFormat(VlkDevice& device, const GrowingArray<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat(VlkDevice& device);

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
		VlkFrameBufferTexture m_frameBufferTexture;

		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;

		GrowingArray<VkFramebuffer> m_swapChainFramebuffers;

		uint32_t m_currentImageIndex = 0;
		uint32_t m_currentFrame = 0;
		uint32_t m_imageCount = 0;

		VkRenderPass m_finalRenderPass = VK_NULL_HANDLE;

	};
}

