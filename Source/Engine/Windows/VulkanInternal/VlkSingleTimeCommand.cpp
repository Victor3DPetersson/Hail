#include "Engine_PCH.h"
#include "VlkTextureCreationFunctions.h"
#include "VlkDevice.h"
#include "VlkSingleTimeCommand.h"

using namespace Hail;

// TODO remove this file

uint32 g_singleTimeCommandCounter = 0u;

VkCommandBuffer Hail::BeginSingleTimeCommands(VlkDevice& device, VkCommandPool commandPool)
{
	H_ASSERT(g_singleTimeCommandCounter == 0, "Must end a command buffer before continuing with a new one.");
	g_singleTimeCommandCounter++;

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Hail::EndSingleTimeCommands(VlkDevice& device, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device.GetDevice(), commandPool, 1, &commandBuffer);
	g_singleTimeCommandCounter--;
	H_ASSERT(g_singleTimeCommandCounter == 0, "Must end a command buffer before continuing with a new one.");
}
