#include "Engine_PCH.h"
#include "RenderContext.h"

#include "FrameBufferTexture.h"
#include "Resources\BufferResource.h"
#include "Resources\ResourceManager.h"
#include "Resources\MaterialManager.h"
#include "Resources\RenderingResourceManager.h"
#include "SwapChain.h"
#include "MathUtils.h"

using namespace Hail;

RenderContext::RenderContext(RenderingDevice* device, ResourceManager* pResourceManager)
	: m_pDevice(device)
	, m_pResourceManager(pResourceManager)
	, m_currentState(eContextState::TransitionBetweenStates)
	, m_pCurrentCommandBuffer(nullptr)
	, m_pBoundMaterial(nullptr)
	, m_pBoundMaterialPipeline(nullptr)
	, m_pBoundVertexBuffer(nullptr)
	, m_pBoundIndexBuffer(nullptr)
	, m_boundMaterialType(eMaterialType::COUNT)
{
	m_pBoundTextures.Fill(nullptr);
	m_pBoundStructuredBuffers.Fill(nullptr);
	m_pBoundUniformBuffers.Fill(nullptr);
	m_pBoundFrameBuffers.Fill(nullptr);
	m_pGraphicsCommandBuffers.Fill(nullptr);
}

void RenderContext::Init()
{
	for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		m_pGraphicsCommandBuffers[i] = CreateCommandBufferInternal(m_pDevice, eContextState::Graphics);
	}
}


void RenderContext::SetBufferAtSlot(BufferObject* pBuffer, uint32 slot)
{
	if (pBuffer->GetProperties().type == eBufferType::uniform)
	{
		m_pBoundUniformBuffers[slot] = pBuffer;
	}
	else if (pBuffer->GetProperties().type == eBufferType::structured)
	{
		m_pBoundStructuredBuffers[slot] = pBuffer;
	}
}

void RenderContext::SetTextureAtSlot(TextureView* pTexture, uint32 slot)
{
	m_pBoundTextures[slot] = pTexture;
}

void RenderContext::BindMaterial(Material* pMaterial)
{
	// TODO: Check shaders expected outputs and make sure it matches the bound framebuffers below and assert if not matching, 
	// pMaterial->m_pPipeline->m_pTypeDescriptor->m_expectedShaderData
	// Not set up yet, so only assert if nothing is bound at slot 0;
	H_ASSERT(m_pCurrentCommandBuffer && m_pCurrentCommandBuffer->m_contextState == eContextState::Graphics);

	if (m_pBoundMaterialPipeline && m_pBoundMaterialPipeline == pMaterial->m_pPipeline)
		return;

	H_ASSERT(m_pBoundFrameBuffers[0], "No framebuffer bound");

	if (!BindMaterialInternal(pMaterial->m_pPipeline))
	{
		m_pBoundMaterial = m_pResourceManager->GetMaterialManager()->GetDefaultMaterial(pMaterial->m_pPipeline->m_type);
		m_pBoundMaterialPipeline = m_pBoundMaterial->m_pPipeline;
		H_ASSERT(BindMaterialInternal(m_pBoundMaterialPipeline));
	}
	else
	{
		m_pBoundMaterial = pMaterial;
		m_pBoundMaterialPipeline = pMaterial->m_pPipeline;
	}
}

void RenderContext::BindMaterial(Pipeline* pPipeline)
{
	H_ASSERT(m_pCurrentCommandBuffer && m_pCurrentCommandBuffer->m_contextState == eContextState::Graphics);

	if (m_pBoundMaterialPipeline && m_pBoundMaterialPipeline == pPipeline)
		return;

	m_pBoundMaterial = nullptr;
	H_ASSERT(m_pBoundFrameBuffers[0], "No framebuffer bound");
	if (!BindMaterialInternal(pPipeline))
	{
		m_pBoundMaterialPipeline = m_pResourceManager->GetMaterialManager()->GetDefaultMaterial(pPipeline->m_type)->m_pPipeline;
		H_ASSERT(BindMaterialInternal(m_pBoundMaterialPipeline));
	}
	else
	{
		m_pBoundMaterialPipeline = pPipeline;
	}
}

void RenderContext::BindFrameBufferAtSlot(FrameBufferTexture* pFrameBuffer, uint32 bindSlot)
{
	H_ASSERT(bindSlot == 0, "No more bind points supported yet");
	m_pBoundFrameBuffers[bindSlot] = pFrameBuffer;
}

void Hail::RenderContext::BindVertexBuffer(BufferObject* pVertexBufferToBind, BufferObject* pIndexBufferToBind)
{
	H_ASSERT(m_pCurrentCommandBuffer && m_currentState == eContextState::Graphics, "Binding a Vertex buffer without recording a graphics pass.");

	if (m_pBoundVertexBuffer == pVertexBufferToBind)
		return;

	H_ASSERT(pVertexBufferToBind->GetProperties().type == eBufferType::vertex, "Wrong buffer type bound.");
	if (pIndexBufferToBind)
		H_ASSERT(pIndexBufferToBind->GetProperties().type == eBufferType::index, "Wrong buffer type bound.");
	m_pBoundVertexBuffer = pVertexBufferToBind;
	m_pBoundIndexBuffer = pIndexBufferToBind;
	BindVertexBufferInternal();
}

void RenderContext::SetPipelineState(Pipeline* pPipeline)
{
	if (!pPipeline->m_pTypeDescriptor)
	{
		H_ERROR(StringL::Format("No bound type data for the pipeline that is being set %u", pPipeline->m_sortKey));
		return;
	}
	// TODO: move this to the context instead of the indirection
	m_pResourceManager->GetMaterialManager()->BindPipelineToContext(pPipeline, this);
}

void Hail::RenderContext::SetPushConstantValue(void* pPushConstant)
{
	H_ASSERT(m_pCurrentCommandBuffer && m_currentState == eContextState::Graphics || m_currentState == eContextState::Compute);
	SetPushConstantInternal(pPushConstant);
}

void Hail::RenderContext::ClearBoundFrameBuffers()
{
	H_ASSERT(m_pCurrentCommandBuffer && m_pCurrentCommandBuffer->m_contextState == eContextState::Graphics);
	H_ASSERT(m_pBoundMaterialPipeline && m_pBoundFrameBuffers[0], "Must have bound a material and a renderpass to be able to clear it");

	// TODO: add support for several clears in a go
	ClearFrameBufferInternal(m_pBoundFrameBuffers[0]);
}

void RenderContext::DeleteFramebuffer(FrameBufferTexture* pFrameBufferToDelete)
{
	//TODO:
}

void RenderContext::DeleteMaterial(Material* pMaterialToDelete)
{
	//TODO:
}

TextureView* RenderContext::GetBoundTextureAtSlot(uint32 slot)
{
	return m_pBoundTextures[slot];
}

BufferObject* RenderContext::GetBoundStructuredBufferAtSlot(uint32 slot)
{
	return m_pBoundStructuredBuffers[slot];
}

BufferObject* RenderContext::GetBoundUniformBufferAtSlot(uint32 slot)
{
	return m_pBoundUniformBuffers[slot];
}

void RenderContext::UploadDataToBuffer(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData)
{
    UploadDataToBufferInternal(pBuffer, pDataToUpload, sizeOfUploadedData);
}

void RenderContext::UploadDataToTexture(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel)
{
	UploadDataToTextureInternal(pTexture, pDataToUpload, mipLevel);
}

void RenderContext::TransferFramebufferLayout(FrameBufferTexture* pTextureToTransfer, eFrameBufferLayoutState colorState, eFrameBufferLayoutState depthState)
{
	uint32 currentFlameInFlight = m_pResourceManager->GetSwapChain()->GetFrameInFlight();

	if (pTextureToTransfer->m_pTextureResource[currentFlameInFlight] && pTextureToTransfer->m_currentColorLayoutState[currentFlameInFlight] != colorState)
	{
		TransferFramebufferLayoutInternal(pTextureToTransfer->m_pTextureResource[currentFlameInFlight], pTextureToTransfer->m_currentColorLayoutState[currentFlameInFlight], colorState);
		pTextureToTransfer->m_currentColorLayoutState[currentFlameInFlight] = colorState;
	}
	if (pTextureToTransfer->m_pDepthTextureResource[currentFlameInFlight] && pTextureToTransfer->m_currentDepthLayoutState[currentFlameInFlight] != depthState)
	{
		TransferFramebufferLayoutInternal(pTextureToTransfer->m_pDepthTextureResource[currentFlameInFlight], pTextureToTransfer->m_currentDepthLayoutState[currentFlameInFlight], depthState);
		pTextureToTransfer->m_currentDepthLayoutState[currentFlameInFlight] = depthState;
	}
}

void Hail::RenderContext::RenderMeshlets(glm::uvec3 dispatchSize)
{
	uint32 minDispatchSize = Math::Min(dispatchSize.x, Math::Min(dispatchSize.y, dispatchSize.z));
	H_ASSERT(minDispatchSize != 0u, "A dispatch of 0 is not allowed in any dimension");
}

void Hail::RenderContext::RenderSprites(uint32 numberOfInstances, uint32 offset)
{
}

void RenderContext::StartGraphicsPass()
{
	H_ASSERT(m_currentState == eContextState::TransitionBetweenStates, "Wrong context state for starting a graphics pass.");
	m_currentlyBoundPipeline = MAX_UINT;
	const uint32 frameInFlight = m_pResourceManager->GetSwapChain()->GetFrameInFlight(); 
	m_pCurrentCommandBuffer = m_pGraphicsCommandBuffers[frameInFlight];
	m_pCurrentCommandBuffer->BeginBuffer();
	m_currentState = eContextState::Graphics;
}

void RenderContext::EndGraphicsPass()
{
	H_ASSERT(m_currentState == eContextState::Graphics, "Wrong context state for ending the graphics pass.");
	H_ASSERT(m_boundMaterialType != eMaterialType::COUNT, "Wrong material context state for ending the graphics pass.");
	EndRenderPass();

	if (m_boundMaterialType == eMaterialType::FULLSCREEN_PRESENT_LETTERBOX)
	{
		SubmitFinalFrameCommandBuffer();
		m_pCurrentCommandBuffer->m_bIsRecording = false;
	}
	else
	{
		m_pCurrentCommandBuffer->EndBuffer(false);
	}

	m_pBoundMaterial = nullptr;
	m_pBoundMaterialPipeline = nullptr;
	m_pBoundVertexBuffer = nullptr;

	m_pCurrentCommandBuffer = nullptr;
	m_currentlyBoundPipeline = MAX_UINT;
	m_currentState = eContextState::TransitionBetweenStates;
	m_boundMaterialType = eMaterialType::COUNT;
}

CommandBuffer::CommandBuffer(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer) :
	m_contextState(contextStateForCommandBuffer),
	m_pDevice(pDevice),
	m_bIsRecording(false)
{
}

CommandBuffer::~CommandBuffer()
{
}

void Hail::CommandBuffer::BeginBuffer()
{
	H_ASSERT(m_bIsRecording == false, "Buffer can not begin recording twice");
	m_bIsRecording = true;
	BeginBufferInternal();
}

void Hail::CommandBuffer::EndBuffer(bool bDestroyBufferData)
{
	if (!bDestroyBufferData)
		H_ASSERT(m_bIsRecording, "Buffer can not end if it is not recording unless it is being destroyed.");

	EndBufferInternal(bDestroyBufferData);
	m_bIsRecording = false;
}

// Transfer pass uses a short lived command buffer, might be a dedicated command buffer in the future
void RenderContext::StartTransferPass()
{
	H_ASSERT(m_currentState == eContextState::TransitionBetweenStates, "End the current pass before starting a new one.");
	m_currentState = eContextState::Transfer;

	m_pCurrentCommandBuffer = CreateCommandBufferInternal(m_pDevice, eContextState::Transfer);
	m_pCurrentCommandBuffer->BeginBuffer();
}

void RenderContext::EndTransferPass()
{
	H_ASSERT(m_currentState == eContextState::Transfer, "Should be in a transfer state when ending the transfer pass.");
	H_ASSERT(m_pCurrentCommandBuffer);
	H_ASSERT(m_currentState == eContextState::Transfer);
	m_pCurrentCommandBuffer->EndBuffer(true);
	SAFEDELETE(m_pCurrentCommandBuffer);
	m_currentState = eContextState::TransitionBetweenStates;

	for (uint32 i = 0; i < m_stagingBuffers.Size(); i++)
	{
		m_stagingBuffers[i]->CleanupResource(m_pDevice);
		SAFEDELETE(m_stagingBuffers[i]);
	}
	m_stagingBuffers.RemoveAll();
}

