#include "Engine_PCH.h"
#include "VlkSwapChain.h"
#include "VlkTextureCreationFunctions.h"
#include "MathUtils.h"

#include "DebugMacros.h"
#include "../Windows_ApplicationWindow.h"
#include "VlkDevice.h"
#include "HailEngine.h"

using namespace Hail;


Hail::VlkSwapChain::VlkSwapChain() : SwapChain()
{
	m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
}

void Hail::VlkSwapChain::Init(RenderingDevice* renderDevice)
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(renderDevice);
	m_frameBufferTexture.SetName("SwapchainFrameBuffer");
	CreateSwapChain(device);
	CreateImageViews(device);
	CreateRenderPass(device);
	CreateFramebuffers(device);
}

bool VlkSwapChain::FrameStart(VlkDevice& device, VkFence* inFrameFences, VkSemaphore* imageAvailableSemaphores)
{
	bool resizedSwapChain = m_resizeSwapChain;
	vkWaitForFences(device.GetDevice(), 1, &inFrameFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	VkResult result = VK_SUCCESS;
	if (!m_resizeSwapChain)
	{
		result = vkAcquireNextImageKHR(device.GetDevice(), m_swapChain, 10000, imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_currentImageIndex);
	}
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_resizeSwapChain)
	{
		RecreateSwapchain(device, inFrameFences, imageAvailableSemaphores);
		resizedSwapChain = true;
		m_resizeSwapChain = false;
		result = VK_SUCCESS;
	}
	else if (result != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to acquire swap chain image");
#endif
	}
	vkResetFences(device.GetDevice(), 1, &inFrameFences[m_currentFrame]);
	return resizedSwapChain;
}

void VlkSwapChain::FrameEnd(VkSemaphore* endSemaphore, VkQueue presentQueue)
{

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = endSemaphore;

	VkSwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_currentImageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(presentQueue, &presentInfo);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMESINFLIGHT;
}


void VlkSwapChain::RecreateSwapchain(VlkDevice& device, VkFence* inFrameFences, VkSemaphore* imageAvailableSemaphores)
{

	glm::uvec2 resolution = Hail::GetApplicationWIndow()->GetWindowResolution();

	vkDeviceWaitIdle(device.GetDevice());
	CleanupSwapchain(device);
	CreateSwapChain(device);
	CreateImageViews(device);
	CreateFramebuffers(device);
	vkDeviceWaitIdle(device.GetDevice());
	vkResetFences(device.GetDevice(), 1, &inFrameFences[m_currentFrame]);
	vkAcquireNextImageKHR(device.GetDevice(), m_swapChain, 10000, imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_currentImageIndex);
}

void Hail::VlkSwapChain::DestroySwapChain(RenderingDevice* renderDevice)
{
	VlkDevice& device = *reinterpret_cast<VlkDevice*>(renderDevice);
	CleanupSwapchain(device);
	vkDestroyRenderPass(device.GetDevice(), m_finalRenderPass, nullptr);
}

FrameBufferTexture* Hail::VlkSwapChain::GetFrameBufferTexture()
{
	return &m_frameBufferTexture;
}

void VlkSwapChain::CleanupSwapchain(VlkDevice& device)
{
	for (size_t i = 0; i < m_swapChainFramebuffers.Size(); i++)
	{
		vkDestroyFramebuffer(device.GetDevice(), m_swapChainFramebuffers[i], nullptr);
	}
	vkDestroySwapchainKHR(device.GetDevice(), m_swapChain, nullptr);
	m_swapChainFramebuffers.DeleteAllAndDeinit();
	m_frameBufferTexture.ClearResources(&device, true);
}

void VlkSwapChain::CreateSwapChain(VlkDevice& device)
{
	SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport(device.GetPhysicalDevice());

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = device.GetPresentSurface();

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = device.FindQueueFamilies(device.GetPhysicalDevice());
	uint32_t queueFamilyIndices[] = { indices.graphicsAndComputeFamily, indices.presentFamily };

	if (indices.graphicsAndComputeFamily != indices.presentFamily) 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(device.GetDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create swap chain!");
#endif
	}
	vkGetSwapchainImagesKHR(device.GetDevice(), m_swapChain, &imageCount, nullptr);
	vkGetSwapchainImagesKHR(device.GetDevice(), m_swapChain, &imageCount, m_frameBufferTexture.m_textureImage);
	m_imageCount = imageCount;
	m_swapChainImageFormat = surfaceFormat.format;
	m_frameBufferTexture.SetTextureFormat(ToInternalFromVkFormat(m_swapChainImageFormat));
	m_swapChainExtent = extent;
	m_windowResolution = { extent.width, extent.height };
	m_frameBufferTexture.SetResolution(m_windowResolution);
	CalculateRenderResolution();
}

VkSurfaceFormatKHR VlkSwapChain::ChooseSwapSurfaceFormat(const GrowingArray<VkSurfaceFormatKHR>& availableFormats)
{
	unsigned short numberOfFormats = availableFormats.Size();
	for (size_t i = 0; i < numberOfFormats; i++)
	{
		if (availableFormats[i].format == VK_FORMAT_R8G8B8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormats[i];
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR VlkSwapChain::ChooseSwapPresentMode(const GrowingArray<VkPresentModeKHR>& availablePresentModes)
{
	unsigned short numberOfModes = availablePresentModes.Size();
	for (size_t i = 0; i < numberOfModes; i++) 
	{
		if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentModes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VlkSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT_MAX) 
	{
		return capabilities.currentExtent;
	}
	else
	{
		glm::uvec2 resolution = Hail::GetApplicationWIndow()->GetWindowResolution();
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(resolution.x),
			static_cast<uint32_t>(resolution.y)
		};

		actualExtent.width = Math::Clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = Math::Clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

VkFramebuffer Hail::VlkSwapChain::GetFrameBuffer(uint32_t index)
{
	return m_swapChainFramebuffers[index];
}

VkFormat Hail::VlkSwapChain::FindSupportedFormat(VlkDevice& device, const GrowingArray<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (uint32_t i = 0; i < candidates.Size(); i++) 
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(device.GetPhysicalDevice(), candidates[i], &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
		{
			return candidates[i];
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
		{
			return candidates[i];
		}
#ifdef DEBUG
		throw std::runtime_error("failed to find supported depth format!");
#endif
	}
}

VkFormat Hail::VlkSwapChain::FindDepthFormat(VlkDevice& device)
{
	return FindSupportedFormat(device,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}



void VlkSwapChain::CreateImageViews(VlkDevice& device)
{
	for (size_t i = 0; i < m_imageCount; i++)
	{
		m_frameBufferTexture.m_textureView[i] = CreateImageView(device, m_frameBufferTexture.m_textureImage[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}


void VlkSwapChain::CreateFramebuffers(VlkDevice& device)
{
	m_swapChainFramebuffers.InitAndFill(m_imageCount);
	for (size_t i = 0; i < m_imageCount; i++)
	{
		VkImageView attachments[1] = { m_frameBufferTexture.m_textureView[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_finalRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.GetDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) 
		{
#ifdef DEBUG
			throw std::runtime_error("failed to create framebuffer!");
#endif
		}
	}
}

void Hail::VlkSwapChain::CreateRenderPass(VlkDevice& device)
{
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = nullptr;

	VkAttachmentDescription attachments[1] = { colorAttachment };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device.GetDevice(), &renderPassInfo, nullptr, &m_finalRenderPass) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create render pass!");
#endif
	}
}

