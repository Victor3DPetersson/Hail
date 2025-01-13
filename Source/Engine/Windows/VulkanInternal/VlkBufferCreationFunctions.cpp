#include "Engine_PCH.h"
#include "VlkTextureCreationFunctions.h"
#include "VlkDevice.h"
#include "VlkSingleTimeCommand.h"

#include "vk_mem_alloc.h"

// TODO remove this file

namespace Hail
{
	bool CreateBuffer(VlkDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vmaInfo{};
		vmaInfo.usage = VMA_MEMORY_USAGE_AUTO;
		
		vmaInfo.flags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		vmaInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		vmaInfo.pool = VK_NULL_HANDLE;

		if (vkCreateBuffer(device.GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			return false;
#ifdef DEBUG
			throw std::runtime_error("failed to create buffer!");
#endif
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.GetDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(device, memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device.GetDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			return false;
#ifdef DEBUG
			throw std::runtime_error("failed to allocate buffer memory!");
#endif
		}

		vkBindBufferMemory(device.GetDevice(), buffer, bufferMemory, 0);
		return true;
	}


	void CopyBuffer(VlkDevice& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue, VkCommandPool commandPool)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(device, commandBuffer, queue, commandPool);
	}
	
}