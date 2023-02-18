//Interface for the entire engine
#pragma once

#include "StartupAttributes.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{
	struct QueueFamilyIndices 
	{
		uint32_t graphicsFamily = INVALID_UINT;
		uint32_t presentFamily = INVALID_UINT;

		bool IsComplete() 
		{
			return graphicsFamily != INVALID_UINT && presentFamily != INVALID_UINT;
		}
	};

	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR capabilities;
		GrowingArray<VkSurfaceFormatKHR> formats;
		GrowingArray<VkPresentModeKHR> presentModes;
	};

	class VlkDevice
	{
	public:
		bool CreateInstance();
		void DestroyDevice();
		VkInstance& GetInstance() { return m_vkInstance; };
		VkDevice& GetDevice() { return m_device; };
		VkSurfaceKHR& GetPresentSurface() { return m_surface; }
		VkPhysicalDevice& GetPhysicalDevice() { return m_physicalDevice; }
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	private:
		bool CheckValidationLayerSupport();
		bool CheckRequiredExtensions();
		void SetupDebugMessenger();
		void PickPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		
		void CreateLogicalDevice();
		void CreateWindowsSurface();

		VkInstance m_vkInstance = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;


#ifdef DEBUG
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
#endif

	};

}

