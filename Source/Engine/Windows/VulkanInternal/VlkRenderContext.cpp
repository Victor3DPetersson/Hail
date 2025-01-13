#include "Engine_PCH.h"

#include "VlkRenderContext.h"

#include "Resources\ResourceManager.h"
#include "Resources\RenderingResourceManager.h"
#include "Resources\BufferResource.h"

#include "VlkDevice.h"
#include "Rendering\SwapChain.h"
#include "Resources\Vulkan\VlkBufferResource.h"

using namespace Hail;

Hail::VlkRenderContext::VlkRenderContext(RenderingDevice* pDevice, ResourceManager* pResourceManager) :RenderContext(pDevice, pResourceManager)
{
}

CommandBuffer* Hail::VlkRenderContext::CreateCommandBufferInternal(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer, bool bIsTempCommandBuffer)
{
    return new VlkCommandBuffer(pDevice, contextStateForCommandBuffer, true);
}

void Hail::VlkRenderContext::UploadDataToBufferInternal(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData)
{
    VlkDevice* p_vlkDevice = (VlkDevice*)m_pDevice;
    const uint32 frameInFlight = m_pResourceManager->GetSwapChain()->GetFrameInFlight();

    H_ASSERT(m_pCurrentCommandBuffer);

    VkCommandBuffer cmdBuffer = ((VlkCommandBuffer*)m_pCurrentCommandBuffer)->m_commandBuffer;

    H_ASSERT(m_currentState == eContextState::Transfer);

    if (pBuffer->UsesPersistentMapping(m_pDevice, frameInFlight))
    {
        // Allocation ended up in a mappable memory and is already mapped - write to it directly.
        // [Executed in runtime]:
        m_pResourceManager->GetRenderingResourceManager()->UploadMemoryToBuffer(pBuffer, pDataToUpload, sizeOfUploadedData);

        VkBufferMemoryBarrier bufMemBarrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
        bufMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        bufMemBarrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
        bufMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier.buffer = ((VlkBufferObject*)pBuffer)->GetBuffer(frameInFlight);
        bufMemBarrier.offset = 0;
        bufMemBarrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            0, 0, nullptr, 1, &bufMemBarrier, 0, nullptr);
    }
    else
    {
        // Allocation ended up in a non-mappable memory - a transfer using a staging buffer is required.
        VkBufferCreateInfo stagingBufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        stagingBufCreateInfo.size = pBuffer->GetBufferSize();
        stagingBufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo stagingAllocCreateInfo = {};
        stagingAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        stagingAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VkBuffer stagingBuf;
        VmaAllocation stagingAlloc;
        VmaAllocationInfo stagingAllocInfo;
        VkResult result = vmaCreateBuffer(p_vlkDevice->GetMemoryAllocator(), &stagingBufCreateInfo, &stagingAllocCreateInfo,
            &stagingBuf, &stagingAlloc, &stagingAllocInfo);
        H_ASSERT(result == VK_SUCCESS);

        memcpy(stagingAllocInfo.pMappedData, pDataToUpload, sizeOfUploadedData);
        result = vmaFlushAllocation(p_vlkDevice->GetMemoryAllocator(), stagingAlloc, 0, VK_WHOLE_SIZE);
        H_ASSERT(result == VK_SUCCESS);

        VkBufferMemoryBarrier bufMemBarrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
        bufMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        bufMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        bufMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier.buffer = stagingBuf;
        bufMemBarrier.offset = 0;
        bufMemBarrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 1, &bufMemBarrier, 0, nullptr);

        VkBufferCopy bufCopy = {
            0, // srcOffset
            0, // dstOffset,
            sizeOfUploadedData, // size
        };

        vkCmdCopyBuffer(cmdBuffer, stagingBuf, ((VlkBufferObject*)pBuffer)->GetBuffer(frameInFlight), 1, &bufCopy);

        VkBufferMemoryBarrier bufMemBarrier2 = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
        bufMemBarrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        bufMemBarrier2.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT; // We created a uniform buffer
        bufMemBarrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier2.buffer = ((VlkBufferObject*)pBuffer)->GetBuffer(frameInFlight);
        bufMemBarrier2.offset = 0;
        bufMemBarrier2.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            0, 0, nullptr, 1, &bufMemBarrier2, 0, nullptr);
    }
}

Hail::VlkCommandBuffer::VlkCommandBuffer(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer, bool bIsTempCommandBuffer) : 
    CommandBuffer(pDevice, contextStateForCommandBuffer, bIsTempCommandBuffer)
{
    VlkDevice& device = *(VlkDevice*)pDevice; 
    VkCommandPool commandPool = device.GetCommandPool();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, &m_commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = m_contextState == eContextState::Transfer ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

    vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
}

void Hail::VlkCommandBuffer::EndBuffer()
{
    vkEndCommandBuffer(m_commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    // TODO: make this use the correct queue based on the command buffer, when command queue is implemented :D
    VlkDevice& device = *(VlkDevice*)m_pDevice;
    vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.GetGraphicsQueue());

    vkFreeCommandBuffers(device.GetDevice(), device.GetCommandPool(), 1, &m_commandBuffer);
}

