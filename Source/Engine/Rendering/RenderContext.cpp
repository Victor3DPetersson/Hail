#include "Engine_PCH.h"
#include "RenderContext.h"

#include "Resources\BufferResource.h"
#include "Resources\ResourceManager.h"
#include "Resources\MaterialManager.h"
#include "Resources\RenderingResourceManager.h"
#include "SwapChain.h"

using namespace Hail;

Hail::RenderContext::RenderContext(RenderingDevice* device, ResourceManager* pResourceManager)
	: m_pDevice(device)
	, m_pResourceManager(pResourceManager)
	, m_currentState(eContextState::TransitionBetweenStates)
{
	m_pBoundTextures.Fill(nullptr);
	m_pBoundStructuredBuffers.Fill(nullptr);
	m_pBoundUniformBuffers.Fill(nullptr);
}

void Hail::RenderContext::SetBufferAtSlot(BufferObject* pBuffer, uint32 slot)
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

void Hail::RenderContext::SetTextureAtSlot(TextureResource* pTexture, uint32 slot)
{
	m_pBoundTextures[slot] = pTexture;
}

void Hail::RenderContext::SetPipelineState(Pipeline* pPipeline)
{
	if (!pPipeline->m_pTypeDescriptor)
	{
		H_ERROR(StringL::Format("No bound type data for the pipeline that is being set %u", pPipeline->m_sortKey));
		return;
	}

	m_pResourceManager->GetMaterialManager()->BindPipelineToContext(pPipeline, this);
}

Hail::TextureResource* Hail::RenderContext::GetBoundTextureAtSlot(uint32 slot)
{
	return m_pBoundTextures[slot];
}

Hail::BufferObject* Hail::RenderContext::GetBoundStructuredBufferAtSlot(uint32 slot)
{
	return m_pBoundStructuredBuffers[slot];
}

Hail::BufferObject* Hail::RenderContext::GetBoundUniformBufferAtSlot(uint32 slot)
{
	return m_pBoundUniformBuffers[slot];
}

void Hail::RenderContext::UploadDataToBuffer(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData)
{
    UploadDataToBufferInternal(pBuffer, pDataToUpload, sizeOfUploadedData);
}

Hail::CommandBuffer::CommandBuffer(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer, bool bIsTempCommandBuffer) :
	m_bIsTempCommandBuffer(bIsTempCommandBuffer),
	m_contextState(contextStateForCommandBuffer),
	m_pDevice(pDevice),
	m_bIsInitialized(true)
{
}

Hail::CommandBuffer::~CommandBuffer()
{
	H_ASSERT(m_bIsInitialized, "Bug in system");
	m_bIsInitialized = false;
}

// Transfer pass uses a short lived command buffer, might be a dedicated command buffer in the future
void Hail::RenderContext::StartTransferPass()
{
	H_ASSERT(m_currentState == eContextState::TransitionBetweenStates, "End the current pass before starting a new one.");
	m_currentState = eContextState::Transfer;

	m_pCurrentCommandBuffer = CreateCommandBufferInternal(m_pDevice, eContextState::Transfer, true);
}

void Hail::RenderContext::EndTransferPass()
{
	H_ASSERT(m_pCurrentCommandBuffer);
	H_ASSERT(m_currentState == eContextState::Transfer);
	m_pCurrentCommandBuffer->EndBuffer();
	SAFEDELETE(m_pCurrentCommandBuffer);
	m_currentState = eContextState::TransitionBetweenStates;

	for (uint32 i = 0; i < m_stagingBuffers.Size(); i++)
	{
		m_stagingBuffers[i]->CleanupResource(m_pDevice);
		SAFEDELETE(m_stagingBuffers[i]);
	}
	m_stagingBuffers.RemoveAll();
}
