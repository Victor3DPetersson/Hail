#include "Engine_PCH.h"
#include "Windows_Renderer.h"
#include "Utilities.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include <array>
#include <set>
#include <stdio.h>

#include "MathUtils.h"


#include "Windows_ApplicationWindow.h"
#include "HailEngine.h"
#include "ShaderCommons.h"
#include "ShaderManager.h"
#include "Rendering\UniformBufferManager.h"

#include "Rendering\Vertices.h"
#include "Timer.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Hail;

constexpr uint32_t DEVICEEXTENSIONCOUNT = 1;
const char* deviceExtensions[DEVICEEXTENSIONCOUNT] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


#ifdef NDEBUG
const bool enableValidationLayers = false;
constexpr uint32_t REQUIREDEXTENSIONCOUNT = 2;
const char* requiredExtensions[REQUIREDEXTENSIONCOUNT] = {
	"VK_KHR_surface",
	"VK_KHR_win32_surface"
};

#else
#include <iostream> 

const bool enableValidationLayers = true;
constexpr uint32_t REQUIREDEXTENSIONCOUNT = 3;
const char* requiredExtensions[REQUIREDEXTENSIONCOUNT] = { 
	"VK_KHR_surface", 
	"VK_KHR_win32_surface",
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

constexpr uint32_t VALIDATIONLAYERCOUNT = 1;
const char* validationLayers[VALIDATIONLAYERCOUNT] = { "VK_LAYER_KHRONOS_validation" };



static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		// Message is important enough to show
	}

	return VK_FALSE;
}

std::vector<TutorialVertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0
};

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

#endif

bool VlkRenderer::Init(RESOLUTIONS startupResolution, ShaderManager* shaderManager, Timer* timer)
{
	m_timer = timer;
	m_shaderManager = shaderManager;
	m_renderResolution =  ResolutionFromEnum(startupResolution);
	if (!CreateInstance())
	{
		return false;
	}
	SetupDebugMessenger();
	CreateWindowsSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreatGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSyncObjects();
	return true;
}

void VlkRenderer::MainLoop()
{
	vkWaitForFences(m_device, 1, &m_inFrameFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex{};
	VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) 
	{
		m_framebufferResized = false;
		RecreateSwapchain();
	}
	else if (result != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to acquire swap chain image");
#endif
	}

	vkResetFences(m_device, 1, &m_inFrameFences[m_currentFrame]);
	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
	UpdateUniformBuffer(m_currentFrame);
	RecordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);


	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFrameFences[m_currentFrame]) != VK_SUCCESS)
	{
	#ifdef DEBUG
		throw std::runtime_error("failed to submit draw command buffer!");
	#endif
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(m_presentQueue, &presentInfo);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMESINFLIGHT;
}

void VlkRenderer::Cleanup()
{
	vkDeviceWaitIdle(m_device);
	CleanupSwapchain();


	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++) 
	{
		vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
		vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
	}
	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

	vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
	vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
	vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);

	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_inFrameFences[i], nullptr);
	}
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);

	vkDestroyDevice(m_device, nullptr);
#ifdef DEBUG
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
	}
#endif

	vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
	vkDestroyInstance(m_vkInstance, nullptr);
}

void VlkRenderer::InitImGui()
{
#ifdef NDEBUG //early out if not in 
	return;
#endif
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

	VkDescriptorPool imguiPool;
	if (vkCreateDescriptorPool(m_device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create ImGuiDescriptor");
#endif
	}
	// 2: initialize imgui library
	//this initializes the core structures of imgui
	ImGui::CreateContext();


	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_vkInstance;
	init_info.PhysicalDevice = m_physicalDevice;
	init_info.Device = m_device;
	init_info.Queue = m_graphicsQueue;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, m_renderPass);

	////execute a gpu command to upload imgui font textures
	//immediate_submit([&](VkCommandBuffer cmd) {
	//	ImGui_ImplVulkan_CreateFontsTexture(cmd);
	//	});

	////clear font textures from cpu data
	//ImGui_ImplVulkan_DestroyFontUploadObjects();

	////add the destroy the imgui created structures
	//_mainDeletionQueue.push_function([=]() {

	//	vkDestroyDescriptorPool(_device, imguiPool, nullptr);
	//	ImGui_ImplVulkan_Shutdown();
	//	});


}

void VlkRenderer::RecreateSwapchain()
{
	glm::uvec2 resolution = Hail::GetApplicationWIndow()->GetWindowResolution();
	VkExtent2D actualExtent = {
		static_cast<uint32_t>(resolution.x),
		static_cast<uint32_t>(resolution.y)
	};
	while (resolution.x == 0 || resolution.y == 0) {
		resolution = Hail::GetApplicationWIndow()->GetWindowResolution();
	}

	vkDeviceWaitIdle(m_device);
	CleanupSwapchain();
	CreateSwapChain();
	CreateImageViews();
	CreateFramebuffers();
}

void VlkRenderer::CreateShaderObject(CompiledShader& shader)
{
	CreateShaderModule(shader);
}

void VlkRenderer::CleanupSwapchain()
{
	for (size_t i = 0; i < m_swapChainFramebuffers.Size(); i++)
	{
		vkDestroyFramebuffer(m_device, m_swapChainFramebuffers[i], nullptr);
	}
	m_swapChainFramebuffers.DeleteAllAndDeinit();
	for (size_t i = 0; i < m_swapChainImageViews.Size(); i++)
	{
		vkDestroyImageView(m_device, m_swapChainImageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	m_swapChainImageViews.DeleteAllAndDeinit();
	m_swapChainImages.DeleteAllAndDeinit();

}

void Hail::VlkRenderer::UpdateUniformBuffer(uint32_t frameInFlight)
{
	float deltaTime = m_timer->GetDeltaTime();
	float totalTime = m_timer->GetTotalTime();
	TutorialUniformBufferObject ubo;

	ubo.model = glm::rotate(glm::mat4(1.0f), deltaTime * glm::radians(1.0f) + totalTime * 0.15f, glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), m_swapChainExtent.width / (float)m_swapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;
	memcpy(m_uniformBuffersMapped[m_currentFrame], &ubo, sizeof(ubo));
}

bool VlkRenderer::CreateInstance()
{
	if (enableValidationLayers && !CheckValidationLayerSupport()) {
#ifdef DEBUG
		throw std::runtime_error("validation layers requested, but not available!");
#endif
		return false;
	}

	if (!CheckRequiredExtensions())
	{
#ifdef DEBUG
		throw std::runtime_error("Required extensions not available!");
#endif
		return false;
	}
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Slask Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Slask Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = REQUIREDEXTENSIONCOUNT;
	createInfo.ppEnabledExtensionNames = requiredExtensions;
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
#ifdef DEBUG
		createInfo.enabledLayerCount = VALIDATIONLAYERCOUNT;
		createInfo.ppEnabledLayerNames = validationLayers;
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);

    if (result != VK_SUCCESS) 
    { 
		return false;
    }

	return true;
}

bool VlkRenderer::CheckValidationLayerSupport()
{
#ifdef DEBUG
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	VkLayerProperties* availableLayers = new VkLayerProperties[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	uint32_t layersFound = 0;
	std::cout << "available layers:\n";
	for (uint32_t foundLayer = 0; foundLayer < layerCount; foundLayer++)
	{
		for (uint32_t validationLayer = 0; validationLayer < VALIDATIONLAYERCOUNT; validationLayer++) {
			if (strcmp(availableLayers[foundLayer].layerName, availableLayers[validationLayer].layerName) == 0) {
				layersFound++;
				break;
			}
		}
		std::cout << '\t' << availableLayers[foundLayer].layerName << std::endl;
	}

	if (layersFound != VALIDATIONLAYERCOUNT) {
		delete[] availableLayers;
		return false;
	}

	delete[] availableLayers;
#endif
	return true;

}

bool VlkRenderer::CheckRequiredExtensions()
{
	uint32_t extensionCount = 0; //query extension counts on the system
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	GrowingArray<VkExtensionProperties> extensions = GrowingArray<VkExtensionProperties>(extensionCount);
	extensions.Fill();
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.Data());

	uint32_t foundCounter = 0;

#ifdef DEBUG
	std::cout << "available extensions:\n";
#endif
	for (uint32_t extension = 0; extension < extensionCount; extension++)
	{
		for (uint32_t reqExtension = 0; reqExtension < REQUIREDEXTENSIONCOUNT; reqExtension++)
		{
			if (strcmp(extensions[extension].extensionName, requiredExtensions[reqExtension]) == 0)
			{
				foundCounter++;
				break;
			}
		}
#ifdef DEBUG
		std::cout << '\t' << extensions[extension].extensionName << '\n';
#endif
	}
	if (foundCounter != REQUIREDEXTENSIONCOUNT)
	{
		return false;
	}
	return true;
}

void VlkRenderer::SetupDebugMessenger()
{
	if (!enableValidationLayers)
	{
		return;
	}
#ifdef DEBUG
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}

#endif
}

void VlkRenderer::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
	if (deviceCount == 0) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
#endif
	}
	GrowingArray<VkPhysicalDevice> devices =GrowingArray<VkPhysicalDevice>(deviceCount);
	devices.Fill();
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.Data());

	for (uint32_t device = 0; device < deviceCount; device++)
	{
		if (IsDeviceSuitable(devices[device])) 
		{
			m_physicalDevice = devices[device];
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to find a suitable GPU!");
#endif
	}
}

bool VlkRenderer::IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = FindQueueFamilies(device);
	bool extensionsSupported = CheckDeviceExtensionSupport(device);
	bool swapChainAdequate = false;

	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.Empty() && !swapChainSupport.presentModes.Empty();
	}

	return indices.IsComplete() && swapChainAdequate;
}

bool VlkRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	GrowingArray<VkExtensionProperties> availableExtensions = GrowingArray < VkExtensionProperties>(extensionCount);
	availableExtensions.Fill();
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.Data());

	uint32_t foundCounter = 0;
#ifdef DEBUG
	std::cout << "available device extensions:\n";
#endif
	for (uint32_t extension = 0; extension < extensionCount; extension++)
	{
		for (uint32_t reqExtension = 0; reqExtension < DEVICEEXTENSIONCOUNT; reqExtension++)
		{
			if (strcmp(deviceExtensions[reqExtension], availableExtensions[extension].extensionName) == 0)
			{
				foundCounter++;
				break;
			}
		}
#ifdef DEBUG
		std::cout << '\t' << availableExtensions[extension].extensionName << '\n';
#endif
	}
	if (foundCounter != DEVICEEXTENSIONCOUNT)
	{
		return false;
	}
	return true;
}

QueueFamilyIndices VlkRenderer::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	VkQueueFamilyProperties* queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for (uint32_t queues = 0; queues < queueFamilyCount; queues++)
	{
		if (queueFamilies[queues].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = queues;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, queues, m_surface, &presentSupport);
		if (presentSupport)
		{
			indices.presentFamily = queues;
		}
		if (indices.IsComplete()) {
			break;
		}
	}
	delete[] queueFamilies;
	return indices;
}

void VlkRenderer::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

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
	createInfo.surface = m_surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily) 
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

	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create swap chain!");
#endif
	}
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.InitAndFill(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.Data());

	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}

SwapChainSupportDetails VlkRenderer::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.formats.InitAndFill(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.Data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) 
	{
		details.presentModes.InitAndFill(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.Data());
	}

	return details;
}

VkSurfaceFormatKHR VlkRenderer::ChooseSwapSurfaceFormat(const GrowingArray<VkSurfaceFormatKHR>& availableFormats)
{
	unsigned short numberOfFormats = availableFormats.Size();
	for (size_t i = 0; i < numberOfFormats; i++)
	{
		if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormats[i];
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR VlkRenderer::ChooseSwapPresentMode(const GrowingArray<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D VlkRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

void VlkRenderer::CreateImageViews()
{
	m_swapChainImageViews.InitAndFill(m_swapChainImages.Size());

	for (size_t i = 0; i < m_swapChainImages.Size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
		{
#ifdef DEBUG
			throw std::runtime_error("failed to create image views!");
#endif
		}
	}
}

void VlkRenderer::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

	GrowingArray<VkDeviceQueueCreateInfo, uint32_t> queueCreateInfos = GrowingArray<VkDeviceQueueCreateInfo, uint32_t>(2);
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.Add(queueCreateInfo);
	}


	VkPhysicalDeviceFeatures deviceFeatures{};


	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.Data();
	createInfo.queueCreateInfoCount = queueCreateInfos.Size();
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = DEVICEEXTENSIONCOUNT;
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (enableValidationLayers) 
	{
#ifdef DEBUG
		createInfo.enabledLayerCount = VALIDATIONLAYERCOUNT;
		createInfo.ppEnabledLayerNames = validationLayers;
#endif
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create logical device!");
#endif
	}

	vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);

}

void VlkRenderer::CreateWindowsSurface()
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	Windows_ApplicationWindow* appWindow = reinterpret_cast<Windows_ApplicationWindow*> (Hail::GetApplicationWIndow());

	if (appWindow)
	{
		createInfo.hwnd = appWindow->GetWindowHandle();
		createInfo.hinstance = appWindow->GetAppModuleHandle();
	}

	if (vkCreateWin32SurfaceKHR(m_vkInstance, &createInfo, nullptr, &m_surface) != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create window surface!");
#endif
	}
}

void VlkRenderer::CreateRenderPass()
{
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
#ifdef DEBUG
		throw std::runtime_error("failed to create render pass!");
#endif
	}
}

void VlkRenderer::CreatGraphicsPipeline()
{
	VkShaderModule vertShaderModule = nullptr;
	VkShaderModule fragShaderModule = nullptr;

	GrowingArray<CompiledShader>& requiredShaders = *m_shaderManager->GetRequiredShaders();
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
	//TEMPORARY CODE BELOW from tutorial TODO: Remove and rework
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
	viewport.width = (float)m_swapChainExtent.width;
	viewport.height = (float)m_swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapChainExtent;

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

	if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
#ifdef DEBUG
		throw std::runtime_error("failed to create pipeline layout!");
#endif
	}


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	auto bindingDescription = TutorialVertex::getBindingDescription();
	auto attributeDescriptions = TutorialVertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
#ifdef DEBUG
		throw std::runtime_error("failed to create graphics pipeline!");
#endif
	}

	vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void VlkRenderer::CreateFramebuffers()
{
	m_swapChainFramebuffers.InitAndFill(m_swapChainImageViews.Size());

	for (size_t i = 0; i < m_swapChainImageViews.Size(); i++) 
	{
		VkImageView attachments[] = { m_swapChainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) 
		{
#ifdef DEBUG
			throw std::runtime_error("failed to create framebuffer!");
#endif
		}
	}
}

void VlkRenderer::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create command pool!");
#endif
	}

}

void VlkRenderer::CreateCommandBuffers()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = MAX_FRAMESINFLIGHT;

	if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to allocate command buffers!");
#endif
	}
}

void VlkRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
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

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapChainExtent;
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapChainExtent.width);
	viewport.height = static_cast<float>(m_swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	VkBuffer vertexBuffers[] = { m_vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[m_currentFrame], 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to record command buffer!");
#endif
	}
}

void VlkRenderer::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++) 
	{
		if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFrameFences[i]) != VK_SUCCESS) {
#ifdef DEBUG
			throw std::runtime_error("failed to create synchronization objects for a frame!");
#endif
		}
	}

}

VkShaderModule VlkRenderer::CreateShaderModule(CompiledShader& shader)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shader.header.sizeOfShaderData;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.compiledCode);
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
#ifdef DEBUG
		throw std::runtime_error("failed to create shader module!");
#endif
		return nullptr;
	}
	shader.loadState = SHADER_LOADSTATE::UPLOADED_TO_GPU;
	//SAFEDELETE_ARRAY(shader.compiledCode);
	return shaderModule;
}

void VlkRenderer::CreateVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

	CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);
	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VlkRenderer::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

	CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VlkRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create buffer!");
#endif
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to allocate buffer memory!");
#endif
	}

	vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

void VlkRenderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	//TODO: Add a transfer Queue for all short lived transfer operations
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void Hail::VlkRenderer::CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(GetUniformBufferSize(UNIFORM_BUFFERS::TUTORIAL));

	m_uniformBuffers.InitAndFill(MAX_FRAMESINFLIGHT);
	m_uniformBuffersMemory.InitAndFill(MAX_FRAMESINFLIGHT);
	m_uniformBuffersMapped.InitAndFill(MAX_FRAMESINFLIGHT);

	for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++) 
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
		vkMapMemory(m_device, m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
	}
}

void Hail::VlkRenderer::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMESINFLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMESINFLIGHT);
	poolInfo.flags = 0;
	if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create descriptor pool!");
#endif
	}

}

void Hail::VlkRenderer::CreateDescriptorSets()
{
	GrowingArray<VkDescriptorSetLayout> layouts(MAX_FRAMESINFLIGHT, m_descriptorSetLayout, false);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMESINFLIGHT);
	allocInfo.pSetLayouts = layouts.Data(); 
	
	m_descriptorSets.InitAndFill(MAX_FRAMESINFLIGHT);
	if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.Data()) != VK_SUCCESS)
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
		bufferInfo.range = GetUniformBufferSize(UNIFORM_BUFFERS::TUTORIAL);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional
		vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
	}
}


uint32_t VlkRenderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
#ifdef DEBUG
	throw std::runtime_error("failed to find suitable memory type!");
#endif
	return 0;
}

void Hail::VlkRenderer::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) 
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create descriptor set layout!");
#endif
	}
}

