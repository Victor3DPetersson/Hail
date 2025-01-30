#include "Engine_PCH.h"

#include "VlkRenderContext.h"

#include "Resources\ResourceManager.h"
#include "Resources\RenderingResourceManager.h"
#include "Resources\BufferResource.h"

#include "VlkDevice.h"
#include "Rendering\SwapChain.h"
#include "Resources\Vulkan\VlkBufferResource.h"
#include "Resources\Vulkan\VlkTextureResource.h"

using namespace Hail;

namespace Internal
{
    void TransitionImageLayout(VkImage image, bool bHasStencil, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (bHasStencil) 
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else 
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else
        {
            H_ASSERT(false, "Invalid image transition operation");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

    }

    void UploadMemoryToBuffer(BufferObject* pBuffer, void* dataToMap, uint32_t sizeOfData, uint32 offset, VlkDevice* pDevice, uint32 frameInFlight)
    {
        H_ASSERT(pBuffer, "Invalid buffer mapped");
        VlkBufferObject* pVlkBuffer = (VlkBufferObject*)pBuffer;
        H_ASSERT(pBuffer->GetBufferSize() >= sizeOfData + offset, "Invalid offset or size of mapped data");
        if (pVlkBuffer->UsesFrameInFlight())
        {
            void* pMappedData = pVlkBuffer->GetAllocationMappedMemory(frameInFlight).pMappedData;
            memcpy((void*)((uint8*)pMappedData + offset), dataToMap, sizeOfData);
        }
        else
        {
            vmaCopyMemoryToAllocation(pDevice->GetMemoryAllocator(), dataToMap, pVlkBuffer->GetAllocation(frameInFlight), offset, sizeOfData);
        }
        VkResult result = vmaFlushAllocation(pDevice->GetMemoryAllocator(), pVlkBuffer->GetAllocation(frameInFlight), 0, VK_WHOLE_SIZE);
        H_ASSERT(result == VK_SUCCESS);
    }

}

Hail::VlkRenderContext::VlkRenderContext(RenderingDevice* pDevice, ResourceManager* pResourceManager) :RenderContext(pDevice, pResourceManager)
{
}

CommandBuffer* Hail::VlkRenderContext::CreateCommandBufferInternal(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer, bool bIsTempCommandBuffer)
{
    return new VlkCommandBuffer(pDevice, contextStateForCommandBuffer, true);
}

void Hail::VlkRenderContext::UploadDataToBufferInternal(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData)
{
    VlkDevice* pVlkDevice = (VlkDevice*)m_pDevice;
    const uint32 frameInFlight = m_pResourceManager->GetSwapChain()->GetFrameInFlight();

    H_ASSERT(m_pCurrentCommandBuffer);

    VkCommandBuffer cmdBuffer = ((VlkCommandBuffer*)m_pCurrentCommandBuffer)->m_commandBuffer;

    H_ASSERT(m_currentState == eContextState::Transfer);

    if (pBuffer->UsesPersistentMapping(m_pDevice, frameInFlight))
    {
        // Allocation ended up in a mappable memory and is already mapped - write to it directly.
        // [Executed in runtime]:
        Internal::UploadMemoryToBuffer(pBuffer, pDataToUpload, sizeOfUploadedData, 0u, (VlkDevice*)m_pDevice, frameInFlight);

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
        VmaAllocator allocator = pVlkDevice->GetMemoryAllocator();

        VlkBufferObject* vlkBuffer = CreateStagingBufferAndMemoryBarrier(pBuffer->GetBufferSize(), pDataToUpload);

        VkBufferCopy bufCopy = {
            0, // srcOffset
            0, // dstOffset,
            sizeOfUploadedData, // size
        };

        vkCmdCopyBuffer(cmdBuffer, vlkBuffer->GetBuffer(0), ((VlkBufferObject*)pBuffer)->GetBuffer(frameInFlight), 1, &bufCopy);

        VkBufferMemoryBarrier bufMemBarrier2 = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
        bufMemBarrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        bufMemBarrier2.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
        bufMemBarrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufMemBarrier2.buffer = ((VlkBufferObject*)pBuffer)->GetBuffer(frameInFlight);
        bufMemBarrier2.offset = 0;
        bufMemBarrier2.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            0, 0, nullptr, 1, &bufMemBarrier2, 0, nullptr);

        m_stagingBuffers.Add(vlkBuffer);
    }
}

void Hail::VlkRenderContext::UploadDataToTextureInternal(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel)
{
    // TODO: Calculate size based on mip
    const uint32_t imageSize = GetTextureByteSize(pTexture->m_properties);
    H_ASSERT(imageSize);
    VlkDevice* pVlkDevice = (VlkDevice*)m_pDevice;

    H_ASSERT(m_pCurrentCommandBuffer);
    VkCommandBuffer cmdBuffer = ((VlkCommandBuffer*)m_pCurrentCommandBuffer)->m_commandBuffer;
    H_ASSERT(m_currentState == eContextState::Transfer);

    VlkTextureResource* pVlkTexture = (VlkTextureResource*)pTexture;
    
    bool hasStencil = false; // TODO
    // TODO: Check initial state of the texture from the properties or something
    Internal::TransitionImageLayout(pVlkTexture->GetVlkTextureData().textureImage, hasStencil, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdBuffer);

    VlkBufferObject* vlkStagingBuffer = CreateStagingBufferAndMemoryBarrier(imageSize, pDataToUpload);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        pTexture->m_properties.width,
        pTexture->m_properties.height,
        1
    };

    vkCmdCopyBufferToImage(
        cmdBuffer,
        vlkStagingBuffer->GetBuffer(0),
        pVlkTexture->GetVlkTextureData().textureImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    VkImageLayout finalImageLayout{};
    if (pTexture->m_properties.textureUsage == eTextureUsage::Texture)
    {
        finalImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    else
    {
        finalImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    Internal::TransitionImageLayout(pVlkTexture->GetVlkTextureData().textureImage, hasStencil, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, finalImageLayout, cmdBuffer);
    m_stagingBuffers.Add(vlkStagingBuffer);
}

VlkBufferObject* Hail::VlkRenderContext::CreateStagingBufferAndMemoryBarrier(uint32 bufferSize, void* pDataToUpload)
{
    VlkDevice* pVlkDevice = (VlkDevice*)m_pDevice;
    VmaAllocator allocator = pVlkDevice->GetMemoryAllocator();
    VkCommandBuffer cmdBuffer = ((VlkCommandBuffer*)m_pCurrentCommandBuffer)->m_commandBuffer;
    VlkBufferObject* vlkStagingBuffer = new VlkBufferObject();

    BufferProperties stagingBufProperties{};
    stagingBufProperties.elementByteSize = bufferSize;
    stagingBufProperties.numberOfElements = 1;
    stagingBufProperties.usage = eShaderBufferUsage::ReadWrite;
    stagingBufProperties.domain = eShaderBufferDomain::CpuToGpu;
    stagingBufProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
    stagingBufProperties.type = eBufferType::staging;

    H_ASSERT(vlkStagingBuffer->Init(m_pDevice, stagingBufProperties), "Failed to create staging buffer, should not happen");

    Internal::UploadMemoryToBuffer(vlkStagingBuffer, pDataToUpload, bufferSize, 0u, (VlkDevice*)m_pDevice, m_pResourceManager->GetSwapChain()->GetFrameInFlight());

    VkResult result = vmaFlushAllocation(allocator, vlkStagingBuffer->GetAllocation(0), 0, VK_WHOLE_SIZE);
    H_ASSERT(result == VK_SUCCESS);

    VkBufferMemoryBarrier bufMemBarrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
    bufMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    bufMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    bufMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufMemBarrier.buffer = vlkStagingBuffer->GetBuffer(0);
    bufMemBarrier.offset = 0;
    bufMemBarrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 1, &bufMemBarrier, 0, nullptr);

    return vlkStagingBuffer;
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

