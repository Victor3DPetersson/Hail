//Interface for the entire engine
#pragma once

#include "StartupAttributes.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
/* Put this somewhere in a header file and include it alongside (and after) vulkan.h: */
extern PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT_;
// This #define lets you call the function the same way as if it was coming from the vulkan.h header
#define vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT_

#include "Containers\GrowingArray\GrowingArray.h"
#include "Rendering\RenderDevice.h"
namespace Hail
{
	struct QueueFamilyIndices
	{
		uint32_t graphicsAndComputeFamily = INVALID_UINT;
		uint32_t presentFamily = INVALID_UINT;

		bool IsComplete() 
		{
			return graphicsAndComputeFamily != INVALID_UINT && presentFamily != INVALID_UINT;
		}
	};

	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR capabilities; 
		GrowingArray<VkSurfaceFormatKHR> formats;
		GrowingArray<VkPresentModeKHR> presentModes;
	};

	class VlkDevice : public RenderingDevice
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
		VkCommandPool GetCommandPool() { return m_commandPool; }
		VkQueue GetGraphicsQueue() { return m_graphicsQueue; }
		VkQueue GetPresentQueue() { return m_presentQueue; }
		VkQueue GetComputeQueue() { return m_computeQueue; }

	private:
		bool CheckValidationLayerSupport();
		bool CheckRequiredExtensions();
		void SetupDebugMessenger();
		void PickPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		
		void CreateLogicalDevice();
		void CreateWindowsSurface();
		void CreateCommandPool();

		VkInstance m_vkInstance = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;

		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		VkQueue m_presentQueue = VK_NULL_HANDLE;
		VkQueue m_computeQueue = VK_NULL_HANDLE;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;

//#ifdef DEBUG
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
//#endif

	};

}

