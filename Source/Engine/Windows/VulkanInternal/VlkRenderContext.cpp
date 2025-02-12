#include "Engine_PCH.h"

#include "VlkRenderContext.h"

#include "Resources\ResourceManager.h"
#include "Resources\RenderingResourceManager.h"
#include "Resources\BufferResource.h"

#include "VlkDevice.h"
#include "Rendering\SwapChain.h"
#include "Resources\MaterialManager.h"
#include "Resources\Vulkan\VlkBufferResource.h"
#include "Resources\Vulkan\VlkTextureResource.h"
#include "Windows\VulkanInternal\VlkFrameBufferTexture.h"
#include "Resources\Vulkan\VlkMaterial.h"
#include "Windows\VulkanInternal\VlkVertex_Descriptor.h"
#include "VlkSwapChain.h"

using namespace Hail;

namespace Internal
{
    VkImageLayout VkImageLayoutFromLayoutState(eFrameBufferLayoutState state)
    {
        switch (state)
        {
        case eFrameBufferLayoutState::Undefined:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case eFrameBufferLayoutState::ShaderRead:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case eFrameBufferLayoutState::ColorAttachment:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case eFrameBufferLayoutState::DepthAttachment:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
    }

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

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (bHasStencil) 
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
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
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
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

    VkPipelineColorBlendAttachmentState CreateBlendMode(eBlendMode blendMode)
    {
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = (blendMode == eBlendMode::Translucent || blendMode == eBlendMode::Additive) ? VK_TRUE : VK_FALSE;
        switch (blendMode)
        {
        case Hail::eBlendMode::None:
        case Hail::eBlendMode::Cutout:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            break;
        case Hail::eBlendMode::Translucent:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            break;
        case Hail::eBlendMode::Additive:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            break;
        default:
            break;
        }
        return colorBlendAttachment;
    }

    VkShaderModule CreateShaderModule(CompiledShader& shader, VlkDevice& device)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shader.header.sizeOfShaderData;
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.compiledCode);
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {

            // TODO: Throw error and move this to the shader loading stage so we can have an early test for this.
            return nullptr;
        }
        return shaderModule;
    }

    VectorOnStack< VkDescriptorSet, 3> GetMaterialDescriptors(ResourceManager* pResourceManager, VlkPipeline* pVlkPipeline, uint32 frameInFlightIndex)
    {
        VectorOnStack< VkDescriptorSet, 3> descriptorSets;
        VlkMaterialTypeObject* pMaterialType = (VlkMaterialTypeObject*)pResourceManager->GetMaterialManager()->GetTypeData(pVlkPipeline);;

        if (pMaterialType->m_globalSetLayout != VK_NULL_HANDLE)
        {
            descriptorSets.Add(pMaterialType->m_globalDescriptors[frameInFlightIndex]);
        }
        if (pMaterialType->m_typeSetLayout != VK_NULL_HANDLE)
        {
            descriptorSets.Add(pMaterialType->m_typeDescriptors[frameInFlightIndex]);
        }

        return descriptorSets;
    }
}

Hail::VlkRenderContext::VlkRenderContext(RenderingDevice* pDevice, ResourceManager* pResourceManager) :RenderContext(pDevice, pResourceManager)
{
    Init();
    VlkDevice& device = *(VlkDevice*)(m_pDevice);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
    {
        if (vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device.GetDevice(), &fenceInfo, nullptr, &m_inFrameFences[i]) != VK_SUCCESS) 
        {
            H_ASSERT(false, "failed to create synchronization objects for a frame!");
        }
    }
}

void Hail::VlkRenderContext::Cleanup()
{
    VlkDevice* pVlkDevice = (VlkDevice*)m_pDevice;
    for (uint32 i = 0; i < m_pFrameBufferMaterialPipelines.Size(); i++)
    {
        m_pFrameBufferMaterialPipelines[i]->Cleanup(m_pDevice);
        SAFEDELETE(m_pFrameBufferMaterialPipelines[i]);
    }
    m_pFrameBufferMaterialPipelines.RemoveAll();


    for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
    {
        m_pGraphicsCommandBuffers[i]->EndBuffer(true);
        SAFEDELETE(m_pGraphicsCommandBuffers[i]);
        vkDestroySemaphore(pVlkDevice->GetDevice(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(pVlkDevice->GetDevice(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(pVlkDevice->GetDevice(), m_inFrameFences[i], nullptr);
    }
}

CommandBuffer* Hail::VlkRenderContext::CreateCommandBufferInternal(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer)
{
    return new VlkCommandBuffer(pDevice, contextStateForCommandBuffer);
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
    
    const bool bHasStencil = HasStencilComponent(ToVkFormat(pVlkTexture->m_properties.format));
    // TODO: Check initial state of the texture from the properties or something
    Internal::TransitionImageLayout(pVlkTexture->GetVlkTextureData().textureImage, bHasStencil, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdBuffer);

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

    Internal::TransitionImageLayout(pVlkTexture->GetVlkTextureData().textureImage, bHasStencil, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, finalImageLayout, cmdBuffer);
    m_stagingBuffers.Add(vlkStagingBuffer);
}

void Hail::VlkRenderContext::TransferFramebufferLayoutInternal(TextureResource* pTextureToTransfer, eFrameBufferLayoutState sourceState, eFrameBufferLayoutState destinationState)
{
    VlkTextureResource* pVlkTexture = (VlkTextureResource*)pTextureToTransfer;
    const bool bHasStencil = HasStencilComponent(ToVkFormat(pVlkTexture->m_properties.format));
    H_ASSERT(m_pCurrentCommandBuffer);
    VkCommandBuffer cmdBuffer = ((VlkCommandBuffer*)m_pCurrentCommandBuffer)->m_commandBuffer;
    Internal::TransitionImageLayout(pVlkTexture->GetVlkTextureData().textureImage, bHasStencil, 
        Internal::VkImageLayoutFromLayoutState(sourceState), Internal::VkImageLayoutFromLayoutState(destinationState), cmdBuffer);
}

void Hail::VlkRenderContext::RenderMeshlets(glm::uvec3 dispatchSize) 
{
    RenderContext::RenderMeshlets(dispatchSize);
    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    vkCmdDrawMeshTasksEXT(vlkCommandBuffer.m_commandBuffer, dispatchSize.x, dispatchSize.y, dispatchSize.z);
}

bool Hail::VlkRenderContext::BindMaterialInternal(Pipeline* pPipeline)
{
    // Check if we already have this combination ready, otherwise create it.
    const uint32_t currentFrame = m_pResourceManager->GetSwapChain()->GetFrameInFlight();
    for (uint32 i = 0; i < m_pFrameBufferMaterialPipelines.Size(); i++)
    {
        MaterialFrameBufferConnection& vkMatFbPipeline = *(MaterialFrameBufferConnection*)m_pFrameBufferMaterialPipelines[i];

        if (vkMatFbPipeline.m_pBoundFrameBuffer == m_pBoundFrameBuffers[0] && vkMatFbPipeline.m_pMaterialPipeline == pPipeline)
        {
            //TODO: If dirty, check if reload is possible, otherwise use a default
            H_ASSERT(vkMatFbPipeline.m_validator.GetIsFrameDataDirty(currentFrame) == false, "Invalid combination, some resource is dirty");
            BindMaterialFrameBufferConnection(m_pFrameBufferMaterialPipelines[i]);
            return true;
        }
    }

    VlkPipeline* pVlkPipeline = (VlkPipeline*)pPipeline;
    H_ASSERT(pVlkPipeline->m_pipelineLayout, "Uninitialized material used when binding material");

    VlkMaterialFrameBufferConnection* vkMatFbPipeline = (VlkMaterialFrameBufferConnection*)CreateMaterialFrameBufferConnection();
    vkMatFbPipeline->m_pBoundFrameBuffer = m_pBoundFrameBuffers[0];
    vkMatFbPipeline->m_pMaterialPipeline = pPipeline;
    vkMatFbPipeline->m_pipelineLayout = pVlkPipeline->m_pipelineLayout;
    // Create pipeline state    
    if (CreateGraphicsPipeline(*vkMatFbPipeline))
    {
        m_pFrameBufferMaterialPipelines.Add(vkMatFbPipeline);
        BindMaterialFrameBufferConnection(vkMatFbPipeline);
        return true;
    }
    return false;
}

void Hail::VlkRenderContext::ClearFrameBufferInternal(FrameBufferTexture* pFrameBuffer)
{
    H_ASSERT(m_pCurrentCommandBuffer);
    VkCommandBuffer cmdBuffer = ((VlkCommandBuffer*)m_pCurrentCommandBuffer)->m_commandBuffer;

    VectorOnStack<VkClearAttachment, 2> attachmentsToClear;
    VkClearAttachment& colorAttachment = attachmentsToClear.Add();
    colorAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkClearValue colorClearValue{};
    colorClearValue.color = { pFrameBuffer->GetClearColor().x, pFrameBuffer->GetClearColor().y, pFrameBuffer->GetClearColor().z, 1.0 };
    colorAttachment.clearValue = colorClearValue;
    colorAttachment.colorAttachment = 0;

    if (pFrameBuffer->HasDepthAttachment())
    {
        const uint32_t currentFrame = m_pResourceManager->GetSwapChain()->GetFrameInFlight();
        const bool bHasStencil = HasStencilComponent(ToVkFormat(pFrameBuffer->GetDepthTexture(currentFrame)->m_properties.format));
        VkClearAttachment& depthAttachment = attachmentsToClear.Add();
        depthAttachment.aspectMask = bHasStencil ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
        depthAttachment.clearValue.depthStencil = { 1.0f, 0 };
    }
    VkClearRect clearRect{};

    const glm::uvec2 resolution = pFrameBuffer->GetResolution();
    clearRect.rect.extent = { resolution.x, resolution.y };
    clearRect.rect.offset = { 0, 0 };
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    vkCmdClearAttachments(
        cmdBuffer,
        attachmentsToClear.Size(),
        attachmentsToClear.Data(),
        1,
        &clearRect);
}

void Hail::VlkRenderContext::EndRenderPass()
{
    if (m_currentlyBoundPipeline != MAX_UINT)
    {
        VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
        VkCommandBuffer& commandBuffer = vlkCommandBuffer.m_commandBuffer;
        vkCmdEndRenderPass(commandBuffer);
        m_currentlyBoundPipeline = MAX_UINT;
    }
}

void Hail::VlkRenderContext::SubmitFinalFrameCommandBuffer()
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const uint32_t currentFrame = m_pResourceManager->GetSwapChain()->GetFrameInFlight();
    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[currentFrame] };

    VkCommandBuffer cmdBuffer = ((VlkCommandBuffer*)m_pCurrentCommandBuffer)->m_commandBuffer;
    vkEndCommandBuffer(cmdBuffer);
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VlkDevice& device = *(VlkDevice*)(m_pDevice);
    if (vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, m_inFrameFences[currentFrame]) != VK_SUCCESS)
    {
        H_ASSERT(false, "failed to submit draw command buffer!");
    }
    VlkSwapChain* pVlkSwapChain = (VlkSwapChain*)m_pResourceManager->GetSwapChain();
    pVlkSwapChain->FrameEnd(signalSemaphores, device.GetPresentQueue());
}

void Hail::VlkRenderContext::StartFrame()
{
    H_ASSERT(m_currentState == eContextState::TransitionBetweenStates && m_pCurrentCommandBuffer == nullptr);
    m_pBoundTextures.Fill(nullptr);
    m_pBoundStructuredBuffers.Fill(nullptr);
    m_pBoundUniformBuffers.Fill(nullptr);
    m_pBoundFrameBuffers.Fill(nullptr);

    VlkDevice& device = *(VlkDevice*)(m_pDevice);
    VlkSwapChain* pVlkSwapChain = (VlkSwapChain*)m_pResourceManager->GetSwapChain();
    if (pVlkSwapChain->FrameStart(device, m_inFrameFences, m_imageAvailableSemaphores))
    {
        //Swapchain has been resized
    }
}

RenderContext::MaterialFrameBufferConnection* Hail::VlkRenderContext::CreateMaterialFrameBufferConnection()
{
    return new VlkRenderContext::VlkMaterialFrameBufferConnection();
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

bool Hail::VlkRenderContext::CreateGraphicsPipeline(VlkMaterialFrameBufferConnection& materialFrameBufferConnection)
{
    VlkDevice& device = *(VlkDevice*)(m_pDevice);
    //TODO: make a wireframe toggle for materials
    const eMaterialType materialType = materialFrameBufferConnection.m_pMaterialPipeline->m_type;
    const bool isWireFrame = materialType == eMaterialType::DEBUG_LINES2D || materialType == eMaterialType::DEBUG_LINES3D;

    bool renderDepth = false;
    VkVertexInputBindingDescription vertexBindingDescription = VkVertexInputBindingDescription();
    GrowingArray<VkVertexInputAttributeDescription> vertexAttributeDescriptions(1);
    switch (materialType)
    {
    case eMaterialType::SPRITE:
        vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::SPRITE);
        vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::SPRITE);
        break;
    case eMaterialType::FULLSCREEN_PRESENT_LETTERBOX:
    {
        VkVertexInputAttributeDescription attributeDescription{};
        attributeDescription.binding = 0;
        attributeDescription.location = 0;
        attributeDescription.format = VK_FORMAT_R32_UINT;
        attributeDescription.offset = 0;
        vertexAttributeDescriptions.Add(attributeDescription);
    }
    vertexBindingDescription.binding = 0;
    vertexBindingDescription.stride = sizeof(uint32_t);
    vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    break;
    case eMaterialType::MODEL3D:
        renderDepth = true;
        vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::MODEL);
        vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::MODEL);
        break;
    case eMaterialType::DEBUG_LINES2D:
    case eMaterialType::DEBUG_LINES3D:
        vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::SPRITE);
        vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::SPRITE);
        break;
    default:
        break;
    }

    H_ASSERT(materialFrameBufferConnection.m_pMaterialPipeline->m_pShaders.Size() == 2, "Insufficient amount of shaders in the material");
    VkShaderModule vertShaderModule;
    bool bUsesMeshShaders = false;
    VkShaderModule fragShaderModule;
    for (size_t i = 0; i < materialFrameBufferConnection.m_pMaterialPipeline->m_pShaders.Size(); i++)
    {
        CompiledShader* pShader = materialFrameBufferConnection.m_pMaterialPipeline->m_pShaders[i];
        if ((eShaderType)pShader->header.shaderType == eShaderType::Vertex)
        {
            vertShaderModule = Internal::CreateShaderModule(*pShader, device);
        }
        else if ((eShaderType)pShader->header.shaderType == eShaderType::Fragment)
        {
            fragShaderModule = Internal::CreateShaderModule(*pShader, device);
        }
        else if ((eShaderType)pShader->header.shaderType == eShaderType::Mesh)
        {
            vertShaderModule = Internal::CreateShaderModule(*pShader, device);
            bUsesMeshShaders = true;
        }

    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = bUsesMeshShaders ? VK_SHADER_STAGE_MESH_BIT_EXT : VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    //l vertexInput, inputAssembly, vertexBindingDescriptions and vertexAttributes

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
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.Size());
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.Data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = isWireFrame ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    const glm::uvec2 resolution = materialFrameBufferConnection.m_pBoundFrameBuffer->GetResolution();
    const VkExtent2D passExtent = { resolution.x, resolution.y };
    viewport.width = (float)passExtent.width;
    viewport.height = (float)passExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = passExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Good to turn off for debugging fragment shader bottlenecks
    rasterizer.polygonMode = isWireFrame ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
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

    VkPipelineColorBlendAttachmentState colorBlendAttachment = Internal::CreateBlendMode(materialFrameBufferConnection.m_pMaterialPipeline->m_blendMode);

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

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = renderDepth;
    depthStencil.depthWriteEnable = renderDepth;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = bUsesMeshShaders ? nullptr : &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = bUsesMeshShaders ? nullptr : &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    VlkFrameBufferTexture* pVlkFrameBuffer = (VlkFrameBufferTexture*)materialFrameBufferConnection.m_pBoundFrameBuffer;

    pipelineInfo.layout = materialFrameBufferConnection.m_pipelineLayout;
    pipelineInfo.renderPass = pVlkFrameBuffer->GetVkRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &materialFrameBufferConnection.m_pipeline) != VK_SUCCESS)
    {
        //TODO ASSERT
        return false;
    }

    vkDestroyShaderModule(device.GetDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(device.GetDevice(), vertShaderModule, nullptr);
    return true;
}

void Hail::VlkRenderContext::BindMaterialFrameBufferConnection(MaterialFrameBufferConnection* pConnectionToBind)
{
    H_ASSERT(m_pCurrentCommandBuffer, "No command buffer started.");

    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    VkCommandBuffer& commandBuffer = vlkCommandBuffer.m_commandBuffer;
    if (m_currentlyBoundPipeline == pConnectionToBind->m_pMaterialPipeline->m_sortKey)
    {
        return;
    }

    EndRenderPass();

    VlkMaterialFrameBufferConnection& vkConnectionToBind = *(VlkMaterialFrameBufferConnection*)pConnectionToBind;

    const uint32_t frameInFlightIndex = m_pResourceManager->GetSwapChain()->GetFrameInFlight();

    if (m_boundMaterialType != pConnectionToBind->m_pMaterialPipeline->m_type)
    {
        VlkPipeline* pVlkPipeline = (VlkPipeline*)pConnectionToBind->m_pMaterialPipeline;
        VectorOnStack< VkDescriptorSet, 3> descriptorSets = Internal::GetMaterialDescriptors(m_pResourceManager, pVlkPipeline, frameInFlightIndex);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkConnectionToBind.m_pipelineLayout, 0, descriptorSets.Size(), descriptorSets.Data(), 0, nullptr);
        m_boundMaterialType = vkConnectionToBind.m_pMaterialPipeline->m_type;
    }

    m_currentlyBoundPipeline = pConnectionToBind->m_pMaterialPipeline->m_sortKey;

    MaterialTypeObject* pMaterialTypeObject = pConnectionToBind->m_pMaterialPipeline->m_pTypeDescriptor;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    const glm::uvec2 passResolution = pConnectionToBind->m_pBoundFrameBuffer->GetResolution();
    VkExtent2D extent = { passResolution.x, passResolution.y };
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    //TEMP: Will remove once we set up reload from resource manager
    if (pConnectionToBind->m_pMaterialPipeline->m_type != eMaterialType::FULLSCREEN_PRESENT_LETTERBOX)
    {
        VlkFrameBufferTexture* pVlkFrameBuffer = (VlkFrameBufferTexture*)vkConnectionToBind.m_pBoundFrameBuffer;
        renderPassInfo.renderPass = pVlkFrameBuffer->GetVkRenderPass();
        renderPassInfo.framebuffer = pVlkFrameBuffer->GetVkFrameBuffer(frameInFlightIndex);
    }
    else
    {
        VlkSwapChain* swapChain = (VlkSwapChain*)m_pResourceManager->GetSwapChain();
        renderPassInfo.renderPass = swapChain->GetRenderPass();
        renderPassInfo.framebuffer = swapChain->GetFrameBuffer(swapChain->GetCurrentSwapImageIndex());
    }
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;

    const glm::vec3 mainClearColor = pConnectionToBind->m_pBoundFrameBuffer->GetClearColor();
    VkClearValue mainClearColors[2];
    mainClearColors[0].color = { mainClearColor.x, mainClearColor.y, mainClearColor.z, 1.0f };
    mainClearColors[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = mainClearColors;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)passResolution.x;
    viewport.height = (float)passResolution.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D mainScissor{};
    mainScissor.offset = { 0, 0 };
    mainScissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &mainScissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkConnectionToBind.m_pipeline);
}

Hail::VlkCommandBuffer::VlkCommandBuffer(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer) : 
    CommandBuffer(pDevice, contextStateForCommandBuffer)
{
    VlkDevice& device = *(VlkDevice*)pDevice; 
    VkCommandPool commandPool = device.GetCommandPool();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, &m_commandBuffer);
}

void Hail::VlkCommandBuffer::BeginBufferInternal()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = m_contextState == eContextState::Transfer ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

    vkResetCommandBuffer(m_commandBuffer, 0);

    vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
}

void Hail::VlkCommandBuffer::EndBufferInternal(bool bDestroyBufferData)
{
    VlkDevice& device = *(VlkDevice*)m_pDevice;
    if (m_bIsRecording)
    {
        vkEndCommandBuffer(m_commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffer;

        // TODO: make this use the correct queue based on the command buffer, when command queue is implemented :D
        vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.GetGraphicsQueue());
    }

    if (bDestroyBufferData)
        vkFreeCommandBuffers(device.GetDevice(), device.GetCommandPool(), 1, &m_commandBuffer);
}

void VlkRenderContext::VlkMaterialFrameBufferConnection::Cleanup(RenderingDevice* pDevice)
{
    VlkDevice& vlkDevice = *(VlkDevice*)pDevice;

    if (m_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(vlkDevice.GetDevice(), m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }
    m_validator = ResourceValidator();
}