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
        return VK_IMAGE_LAYOUT_UNDEFINED;
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

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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

    VkFormat GetVlkTypeFromReflectedType(eShaderValueType typeToGetFrom)
    {
        switch (typeToGetFrom)
        {
        case eShaderValueType::none:
        {
            H_ASSERT(false);
            return VK_FORMAT_UNDEFINED;
        }
        case eShaderValueType::int8:
            return VK_FORMAT_R8_SINT;
        case eShaderValueType::uint8:
            return VK_FORMAT_R8_UINT;
        case eShaderValueType::int16:
            return VK_FORMAT_R16_SINT;
        case eShaderValueType::uint16:
            return VK_FORMAT_R16_UINT;
        case eShaderValueType::int32:
            return VK_FORMAT_R32_SINT;
        case eShaderValueType::boolean:
        case eShaderValueType::uint32:
            return VK_FORMAT_R32_UINT;
        case eShaderValueType::int64:
            return VK_FORMAT_R64_SINT;
        case eShaderValueType::uint64:
            return VK_FORMAT_R64_UINT;
        case eShaderValueType::float16:
            return VK_FORMAT_R16_SFLOAT;
        case eShaderValueType::float32:
            return VK_FORMAT_R32_SFLOAT;
        case eShaderValueType::float64:
            return VK_FORMAT_R64_SFLOAT;
        }
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

void Hail::VlkRenderContext::BindMaterialInstance(uint32 materialInstanceIndex)
{
    VlkCommandBuffer* pVlkCommandBfr = (VlkCommandBuffer*)GetCurrentCommandBuffer();
    VkCommandBuffer commandBuffer = pVlkCommandBfr->m_commandBuffer;
    H_ASSERT(m_pBoundMaterial, "No bound material, is this called in a material pipeline pass?");
    VlkMaterial& vlkMaterial = *(VlkMaterial*)m_pBoundMaterial;
    VlkPipeline& vlkPipeline = *(VlkPipeline*)vlkMaterial.m_pPipeline;

    H_ASSERT(vlkMaterial.m_instanceDescriptors.Size() > materialInstanceIndex);

    const uint32 frameInFlight = m_pResourceManager->GetSwapChain()->GetFrameInFlight();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vlkPipeline.m_pipelineLayout, 2, 1, &vlkMaterial.m_instanceDescriptors[materialInstanceIndex].descriptors[frameInFlight], 0, nullptr);
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

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
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

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
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

void Hail::VlkRenderContext::Dispatch(glm::uvec3 dispatchSize)
{
    // TODO lägg till så att jag vet ifall jag har varit i en dispatch för att skapa flera memory barriers. Eller låt resurserna veta om hur dem används. 
    RenderContext::Dispatch(dispatchSize);
    H_ASSERT(m_pBoundMaterialPipeline->m_bIsCompute);
    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;

    vkCmdDispatch(vlkCommandBuffer.m_commandBuffer, dispatchSize.x, dispatchSize.y, dispatchSize.z);

    //VkMemoryBarrier barrier{};
    //barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    //barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    //barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;


    //vkCmdPipelineBarrier(
    //    vlkCommandBuffer.m_commandBuffer,
    //    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    //    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    //    0,
    //    1, &barrier,
    //    0, nullptr,
    //    0, nullptr
    //);
    // Synchronize 
}

void Hail::VlkRenderContext::RenderMeshlets(glm::uvec3 dispatchSize)
{
    RenderContext::RenderMeshlets(dispatchSize);
    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    vkCmdDrawMeshTasksEXT(vlkCommandBuffer.m_commandBuffer, dispatchSize.x, dispatchSize.y, dispatchSize.z);
}

void Hail::VlkRenderContext::RenderFullscreenPass()
{
    H_ASSERT(m_pBoundVertexBuffer == nullptr, "No vertex buffer should be bound for fullscreen rendering.");
    const uint32 frameInFlightIndex = m_pResourceManager->GetSwapChain()->GetFrameInFlight();
    VlkCommandBuffer* pVlkCommandBuffer = (VlkCommandBuffer*)GetCurrentCommandBuffer();
    VkCommandBuffer& commandBuffer = pVlkCommandBuffer->m_commandBuffer;

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void Hail::VlkRenderContext::RenderInstances(uint32 numberOfInstances, uint32 offset)
{
    H_ASSERT(m_pCurrentCommandBuffer && m_currentState == eContextState::Graphics, "Can not render outside of a graphics pass");
    H_ASSERT(m_pBoundMaterial || m_pBoundMaterialPipeline, "Must have bound a material to render instances.");

    bool bDoesNotNeedVertexBuffer = m_boundMaterialType == eMaterialType::CUSTOM;
    bool bHaveABoundVertexBuffer = m_pBoundVertexBuffer;
    H_ASSERT(bDoesNotNeedVertexBuffer != bHaveABoundVertexBuffer, "Must have bound a material to render sprites.");

    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    VkCommandBuffer commandBuffer = vlkCommandBuffer.m_commandBuffer;

    VlkPipeline& vlkPipeline = *(VlkPipeline*)(m_pBoundMaterial ? m_pBoundMaterial->m_pPipeline : m_pBoundMaterialPipeline);

    if (vlkPipeline.m_type == eMaterialType::SPRITE)
    {
        // TODO: use the reflected push constants
        glm::uvec4 pushConstants_instanceID_padding = { offset, 0, 0, 0 };
        vkCmdPushConstants(commandBuffer, vlkPipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::uvec4), &pushConstants_instanceID_padding);
    }
    vkCmdDraw(commandBuffer, 6, numberOfInstances, 0, 0);
}

void Hail::VlkRenderContext::RenderDebugLines(uint32 numberOfLinesToRender)
{
    H_ASSERT(m_pCurrentCommandBuffer && m_currentState == eContextState::Graphics, "Can not render outside of a graphics pass");
    H_ASSERT(m_pBoundMaterialPipeline, "Must have bound a material to render lines.");
    H_ASSERT(m_boundMaterialType == eMaterialType::CUSTOM, "Must have a custom material for lines.");

    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    VkCommandBuffer commandBuffer = vlkCommandBuffer.m_commandBuffer;

    vkCmdDraw(commandBuffer, numberOfLinesToRender, 1, 0u, 0);
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

    VlkMaterialFrameBufferConnection* vkMatFbPipeline = new VlkRenderContext::VlkMaterialFrameBufferConnection();
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
    vkMatFbPipeline->Cleanup(m_pDevice);
    SAFEDELETE(vkMatFbPipeline);
    return false;
}

bool Hail::VlkRenderContext::BindComputePipelineInternal(Pipeline* pPipeline)
{
    // Check if we already have this combination ready, otherwise create it.
    for (uint32 i = 0; i < m_pComputePipelines.Size(); i++)
    {
        VlkComputePipeline& computePipeline = *(VlkComputePipeline*)m_pComputePipelines[i];

        if (computePipeline.m_pMaterialPipeline == pPipeline)
        {
            BindComputePipeline(m_pComputePipelines[i]);
            return true;
        }
    }

    VlkPipeline* pVlkPipeline = (VlkPipeline*)pPipeline;
    H_ASSERT(pVlkPipeline->m_pipelineLayout, "Uninitialized material used when binding material");

    VlkComputePipeline* pComputePipeline = new VlkRenderContext::VlkComputePipeline();;
    pComputePipeline->m_pMaterialPipeline = pPipeline;
    pComputePipeline->m_pipelineLayout = pVlkPipeline->m_pipelineLayout;
    if (CreateComputePipeline(*pComputePipeline))
    {
        m_pComputePipelines.Add(pComputePipeline);
        BindComputePipeline(pComputePipeline);
        return true;
    }
    pComputePipeline->Cleanup(m_pDevice);
    SAFEDELETE(pComputePipeline);
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

void Hail::VlkRenderContext::SetPushConstantInternal(void* pPushConstant)
{
    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    VkCommandBuffer commandBuffer = vlkCommandBuffer.m_commandBuffer;

    VlkPipeline& vlkPipeline = *(VlkPipeline*)m_pBoundMaterialPipeline;

    //TODO: Check shader validation first

    uint32 stage{};
    if (m_currentState == eContextState::Graphics)
    {
        for (uint32 i = 0; i < vlkPipeline.m_pShaders.Size(); i++)
        {
            const eShaderStage shaderType = (eShaderStage)vlkPipeline.m_pShaders[i]->header.shaderType;
            bool bContainsPushConstants = vlkPipeline.m_pShaders[i]->reflectedShaderData.m_pushConstants.Size();

            if (!bContainsPushConstants)
                continue;

            if (shaderType == eShaderStage::Vertex || shaderType == eShaderStage::Mesh)
            {
                stage |= shaderType == eShaderStage::Vertex ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_MESH_BIT_EXT;
            }
            else if (shaderType == eShaderStage::Fragment)
            {
                stage |= VK_SHADER_STAGE_FRAGMENT_BIT;
            }
        }
    }
    else if (m_currentState == eContextState::Compute)
    {
        stage = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    H_ASSERT(stage, "No valid stage found");
    vkCmdPushConstants(commandBuffer, vlkPipeline.m_pipelineLayout, stage, 0, sizeof(glm::uvec4), pPushConstant);
}

void Hail::VlkRenderContext::EndRenderPass()
{
    if (m_currentlyBoundPipeline != MAX_UINT && m_lastBoundShaderStages != ComputeShaderStage)
    {
        VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
        VkCommandBuffer& commandBuffer = vlkCommandBuffer.m_commandBuffer;
        vkCmdEndRenderPass(commandBuffer);
        CleanupAndEndPass();
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

VkAccessFlags LocalAccessMaskFromAccessFlag(eShaderAccessQualifier qualifier)
{
    if (qualifier == eShaderAccessQualifier::ReadOnly)
    {
        return VK_ACCESS_SHADER_READ_BIT;
    }
    else if (qualifier == eShaderAccessQualifier::WriteOnly)
    {
        return VK_ACCESS_SHADER_WRITE_BIT;
    }
    else
    {
        return VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    }
}

void Hail::VlkRenderContext::TransferImageStateInternal(TextureResource* pTexture, eShaderAccessQualifier newState)
{
    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    VkCommandBuffer& commandBuffer = vlkCommandBuffer.m_commandBuffer;

    VlkTextureResource* pVlkTexture = (VlkTextureResource*)pTexture;

    VkImageSubresourceRange imageSubRange{};
    imageSubRange.baseMipLevel = 0;
    imageSubRange.layerCount = 1;
    imageSubRange.levelCount = 1;
    imageSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier imageBarrier{};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.pNext = NULL;
    imageBarrier.srcAccessMask = LocalAccessMaskFromAccessFlag(pVlkTexture->m_accessQualifier);
    imageBarrier.dstAccessMask = LocalAccessMaskFromAccessFlag(newState);
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.subresourceRange = imageSubRange;
    imageBarrier.image = pVlkTexture->GetVlkTextureData().textureImage;
    imageBarrier.oldLayout = pVlkTexture->GetVlkTextureData().imageLayout;

    VkPipelineStageFlags sourceUsage = pVlkTexture->GetVlkTextureData().currentUsage;
    VkPipelineStageFlags destinationUsage;
    if (newState == eShaderAccessQualifier::ReadOnly)
    {
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        destinationUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    else if (newState == eShaderAccessQualifier::ReadWrite)
    {
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        destinationUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    }
    else
    {
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        destinationUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (pVlkTexture->GetVlkTextureData().imageLayout == imageBarrier.newLayout)
    {
        H_ASSERT(pVlkTexture->m_accessQualifier == newState, "Probably a logic error going on here");
        return;
    }

    pVlkTexture->m_accessQualifier = newState;
    pVlkTexture->GetVlkTextureData().imageLayout = imageBarrier.newLayout;
    pVlkTexture->GetVlkTextureData().currentUsage = destinationUsage;

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceUsage,
        destinationUsage,
        0,
        0, NULL,
        0, NULL,
        1, &imageBarrier
    );

}

void Hail::VlkRenderContext::TransferBufferStateInternal(BufferObject* /*pBuffer*/, eShaderAccessQualifier /*newState*/)
{
    //VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    //VkCommandBuffer& commandBuffer = vlkCommandBuffer.m_commandBuffer;

    //VkBufferMemoryBarrier bufferBarrier{};
    //bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    //bufferBarrier.pNext = NULL;
    //bufferBarrier.srcAccessMask = LocalAccessMaskFromAccessFlag(pBuffer->GetAccessQualifier());
    //bufferBarrier.dstAccessMask = LocalAccessMaskFromAccessFlag(newState);
    //bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //bufferBarrier.buffer = yourBuffer;
    //bufferBarrier.offset = 0;
    //bufferBarrier.size = VK_WHOLE_SIZE;

    //vkCmdPipelineBarrier(
    //    commandBuffer,
    //    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    //    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    //    0,
    //    0, NULL,           
    //    1, &bufferBarrier,
    //    0, NULL
    //);
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

VlkBufferObject* Hail::VlkRenderContext::CreateStagingBufferAndMemoryBarrier(uint32 bufferSize, void* pDataToUpload)
{
    VlkDevice* pVlkDevice = (VlkDevice*)m_pDevice;
    VmaAllocator allocator = pVlkDevice->GetMemoryAllocator();
    VkCommandBuffer cmdBuffer = ((VlkCommandBuffer*)m_pCurrentCommandBuffer)->m_commandBuffer;
    VlkBufferObject* vlkStagingBuffer = new VlkBufferObject();

    BufferProperties stagingBufProperties{};
    stagingBufProperties.elementByteSize = bufferSize;
    stagingBufProperties.numberOfElements = 1;
    stagingBufProperties.domain = eShaderBufferDomain::CpuToGpu;
    stagingBufProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
    stagingBufProperties.type = eBufferType::staging;

    H_ASSERT(vlkStagingBuffer->Init(m_pDevice, stagingBufProperties, "Temp Staging"), "Failed to create staging buffer, should not happen");

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

    bool renderDepth = false;
    bool bUseVertexBuffer = true;
    VkVertexInputBindingDescription vertexBindingDescription = VkVertexInputBindingDescription();
    GrowingArray<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
    switch (materialType)
    {
    case eMaterialType::SPRITE:
        vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::SPRITE);
        vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::SPRITE);
        break;
    case eMaterialType::FULLSCREEN_PRESENT_LETTERBOX:
    case eMaterialType::CUSTOM:
    {
        bUseVertexBuffer = materialFrameBufferConnection.m_pMaterialPipeline->m_bUsesVertexBuffer;

        CompiledShader* pVertShader = materialFrameBufferConnection.m_pMaterialPipeline->m_pShaders[0];

        if (bUseVertexBuffer && pVertShader->header.shaderType == (uint32)eShaderStage::Vertex)
        {
            uint32 currentOffset = 0u;
            for (uint32 i = 0; i < pVertShader->reflectedShaderData.m_shaderInputs.Size(); i++)
            {
                const ShaderDecoration& inputDecoration = pVertShader->reflectedShaderData.m_shaderInputs[i];
                VkVertexInputAttributeDescription bindingDescription{};
                bindingDescription.binding = 0u;
                bindingDescription.location = inputDecoration.m_bindingLocation;
                bindingDescription.format = Internal::GetVlkTypeFromReflectedType(inputDecoration.m_valueType);
                bindingDescription.offset = currentOffset;
                vertexAttributeDescriptions.Add(bindingDescription);

                currentOffset += inputDecoration.m_byteSize;
            }

            vertexBindingDescription.binding = 0;
            vertexBindingDescription.stride = currentOffset;
            vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }
    }
    break;
    case eMaterialType::MODEL3D:
        renderDepth = true;
        vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::MODEL);
        vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::MODEL);
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
        if ((eShaderStage)pShader->header.shaderType == eShaderStage::Vertex)
        {
            vertShaderModule = Internal::CreateShaderModule(*pShader, device);
        }
        else if ((eShaderStage)pShader->header.shaderType == eShaderStage::Fragment)
        {
            fragShaderModule = Internal::CreateShaderModule(*pShader, device);
        }
        else if ((eShaderStage)pShader->header.shaderType == eShaderStage::Mesh)
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
    vertexInputInfo.vertexBindingDescriptionCount = bUseVertexBuffer ? 1u : 0u;
    vertexInputInfo.vertexAttributeDescriptionCount = bUseVertexBuffer ? (uint32)vertexAttributeDescriptions.Size() : 0u;
    vertexInputInfo.pVertexBindingDescriptions = bUseVertexBuffer ? &vertexBindingDescription : nullptr;
    vertexInputInfo.pVertexAttributeDescriptions = bUseVertexBuffer ? vertexAttributeDescriptions.Data() : nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = materialFrameBufferConnection.m_pMaterialPipeline->m_bIsWireFrame ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
    rasterizer.polygonMode = materialFrameBufferConnection.m_pMaterialPipeline->m_bIsWireFrame ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
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

bool Hail::VlkRenderContext::CreateComputePipeline(VlkComputePipeline& computePipeline)
{
    VlkDevice& device = *(VlkDevice*)(m_pDevice);
    CompiledShader* pShader = computePipeline.m_pMaterialPipeline->m_pShaders[0];
    H_ASSERT(computePipeline.m_pMaterialPipeline->m_pShaders.Size() == 1 && (eShaderStage)pShader->header.shaderType == eShaderStage::Compute, "Compute pipeline should only have 1 shader");
    VkShaderModule computeShaderModule = Internal::CreateShaderModule(*pShader, device);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = computePipeline.m_pipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    return vkCreateComputePipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline.m_pipeline) == VK_SUCCESS;
}

void Hail::VlkRenderContext::BindMaterialFrameBufferConnection(MaterialFrameBufferConnection* pConnectionToBind)
{
    H_ASSERT(m_pCurrentCommandBuffer, "No command buffer started.");
    if (m_currentlyBoundPipeline == pConnectionToBind->m_pMaterialPipeline->m_sortKey)
    {
        return;
    }

    EndRenderPass();

    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    VkCommandBuffer& commandBuffer = vlkCommandBuffer.m_commandBuffer;

    VlkMaterialFrameBufferConnection& vkConnectionToBind = *(VlkMaterialFrameBufferConnection*)pConnectionToBind;

    const uint32_t frameInFlightIndex = m_pResourceManager->GetSwapChain()->GetFrameInFlight();

    const bool bRebindTypeDescriptors = m_boundMaterialType != pConnectionToBind->m_pMaterialPipeline->m_type || pConnectionToBind->m_pMaterialPipeline->m_type == eMaterialType::CUSTOM;
    if (bRebindTypeDescriptors)
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

void Hail::VlkRenderContext::BindComputePipeline(ComputePipeline* pPipelineToBind)
{
    H_ASSERT(m_pCurrentCommandBuffer, "No command buffer started.");
    if (m_currentlyBoundPipeline == pPipelineToBind->m_pMaterialPipeline->m_sortKey)
        return;

    EndRenderPass();

    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    VkCommandBuffer& commandBuffer = vlkCommandBuffer.m_commandBuffer;
    VlkComputePipeline* pVlkComputePipeline = (VlkComputePipeline*)pPipelineToBind;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pVlkComputePipeline->m_pipeline);

    const uint32_t frameInFlightIndex = m_pResourceManager->GetSwapChain()->GetFrameInFlight();
    VlkPipeline* pVlkPipeline = (VlkPipeline*)pPipelineToBind->m_pMaterialPipeline;
    VectorOnStack< VkDescriptorSet, 3> descriptorSets = Internal::GetMaterialDescriptors(m_pResourceManager, pVlkPipeline, frameInFlightIndex);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pVlkComputePipeline->m_pipelineLayout, 0, descriptorSets.Size(), descriptorSets.Data(), 0, nullptr);
}

void Hail::VlkRenderContext::BindVertexBufferInternal()
{
    VlkCommandBuffer& vlkCommandBuffer = *(VlkCommandBuffer*)m_pCurrentCommandBuffer;
    VkCommandBuffer& commandBuffer = vlkCommandBuffer.m_commandBuffer;

    VkDeviceSize spriteOffsets[] = { 0 };
    if (m_pBoundVertexBuffer)
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &((VlkBufferObject*)m_pBoundVertexBuffer)->GetBuffer(0), spriteOffsets);

    if (m_pBoundIndexBuffer)
        vkCmdBindIndexBuffer(commandBuffer, ((VlkBufferObject*)m_pBoundIndexBuffer)->GetBuffer(0), 0, VK_INDEX_TYPE_UINT32);

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

void Hail::VlkRenderContext::VlkComputePipeline::Cleanup(RenderingDevice* pDevice)
{
    VlkDevice& vlkDevice = *(VlkDevice*)pDevice;
    if (m_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(vlkDevice.GetDevice(), m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }
}
