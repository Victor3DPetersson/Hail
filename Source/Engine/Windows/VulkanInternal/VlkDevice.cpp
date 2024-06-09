#include "Engine_PCH.h"
#include "VlkDevice.h"
#include "Utilities.h"


#include <array>
#include <set>
#include <stdio.h>

#include "MathUtils.h"

#include "DebugMacros.h"

#include "../Windows_ApplicationWindow.h"
#include "HailEngine.h"
#include "Resources\TextureManager.h"

#include "Resources\Vertices.h"
#include "Timer.h"
#include "Resources\ResourceManager.h"

#include "VlkVertex_Descriptor.h"

using namespace Hail;

constexpr uint32_t DEVICEEXTENSIONCOUNT = 1;
const char* deviceExtensions[DEVICEEXTENSIONCOUNT] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
constexpr uint32_t REQUIREDEXTENSIONCOUNT = 2;
const char* requiredExtensions[REQUIREDEXTENSIONCOUNT] = {
	"VK_KHR_surface",
	"VK_KHR_win32_surface"
};

#else
#include <iostream> 

constexpr bool enableValidationLayers = true;
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

	//std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
	{
		Debug_PrintConsoleConstChar(pCallbackData->pMessage);
		// Message is important enough to show
	}

	return VK_FALSE;
}


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
	if (func != nullptr) 
	{
		func(instance, debugMessenger, pAllocator);
	}
}

#endif


void VlkDevice::DestroyDevice()
{
	vkDestroyDevice(m_device, nullptr);
#ifdef DEBUG
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
	}
#endif

	vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
	vkDestroyInstance(m_vkInstance, nullptr);
}


bool VlkDevice::CreateInstance()
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
	appInfo.pApplicationName = "Hail Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Hail Engine";
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
	SetupDebugMessenger();
	CreateWindowsSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();

	Hail::QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
	vkGetDeviceQueue(m_device, indices.graphicsAndComputeFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.graphicsAndComputeFamily, 0, &m_computeQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
	CreateCommandPool();

	return true;
}

bool VlkDevice::CheckValidationLayerSupport()
{
#ifdef DEBUG
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	VkLayerProperties* availableLayers = new VkLayerProperties[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	uint32_t layersFound = 0;
	//Debug_PrintConsoleConstChar("Available layers:");
	for (uint32_t foundLayer = 0; foundLayer < layerCount; foundLayer++)
	{
		for (uint32_t validationLayer = 0; validationLayer < VALIDATIONLAYERCOUNT; validationLayer++) {
			if (strcmp(availableLayers[foundLayer].layerName, availableLayers[validationLayer].layerName) == 0) {
				layersFound++;
				break;
			}
		}
		Debug_PrintConsoleString256(String256::Format("\t%s", availableLayers[foundLayer].layerName));
	}

	if (layersFound != VALIDATIONLAYERCOUNT) {
		delete[] availableLayers;
		return false;
	}

	delete[] availableLayers;
#endif
	return true;

}

bool VlkDevice::CheckRequiredExtensions()
{
	uint32_t extensionCount = 0; //query extension counts on the system
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	GrowingArray<VkExtensionProperties> extensions = GrowingArray<VkExtensionProperties>(extensionCount);
	extensions.Fill();
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.Data());

	uint32_t foundCounter = 0;

	//Debug_PrintConsoleConstChar("Available vKInstance extensions:");
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
		Debug_PrintConsoleString256(String256::Format("\t%s", extensions[extension].extensionName));
	}
	if (foundCounter != REQUIREDEXTENSIONCOUNT)
	{
		return false;
	}
	return true;
}

void VlkDevice::SetupDebugMessenger()
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

void VlkDevice::PickPhysicalDevice()
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

bool VlkDevice::IsDeviceSuitable(VkPhysicalDevice device)
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

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.IsComplete() && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VlkDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	GrowingArray<VkExtensionProperties> availableExtensions = GrowingArray < VkExtensionProperties>(extensionCount);
	availableExtensions.Fill();
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.Data());

	uint32_t foundCounter = 0;
	//Debug_PrintConsoleConstChar("available device extensions:");
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
		Debug_PrintConsoleString256(String256::Format("\t%s", availableExtensions[extension].extensionName));
	}
	if (foundCounter != DEVICEEXTENSIONCOUNT)
	{
		return false;
	}
	return true;
}

QueueFamilyIndices VlkDevice::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	VkQueueFamilyProperties* queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for (uint32_t queues = 0; queues < queueFamilyCount; queues++)
	{
		if ((queueFamilies[queues].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamilies[queues].queueFlags & VK_QUEUE_COMPUTE_BIT))
		{
			indices.graphicsAndComputeFamily = queues;
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



void VlkDevice::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

	GrowingArray<VkDeviceQueueCreateInfo, uint32_t> queueCreateInfos = GrowingArray<VkDeviceQueueCreateInfo, uint32_t>(2);
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsAndComputeFamily, indices.presentFamily };

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
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	//TODO: Add an If debug
	deviceFeatures.fillModeNonSolid = VK_TRUE;

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
}

void VlkDevice::CreateWindowsSurface()
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

void Hail::VlkDevice::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily;
	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
	{
#ifdef DEBUG
		throw std::runtime_error("failed to create command pool!");
#endif
	}
}

SwapChainSupportDetails VlkDevice::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.Prepare(formatCount);
		details.formats.Fill();
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.Data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.Prepare(presentModeCount);
		details.presentModes.Fill();
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.Data());
	}

	return details;
}