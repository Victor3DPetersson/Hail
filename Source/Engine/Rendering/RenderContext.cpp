#include "Engine_PCH.h"
#include "RenderContext.h"

#include "Resources\BufferResource.h"
#include "Resources\ResourceManager.h"
#include "Resources\MaterialManager.h"
#include "Resources\RenderingResourceManager.h"
#include "SwapChain.h"

using namespace Hail;

Hail::RenderContext::RenderContext(ResourceManager* pResourceManager)
	: m_pResourceManager(pResourceManager)
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
	m_pResourceManager->GetRenderingResourceManager()->MapMemoryToBuffer(pBuffer, pDataToUpload, sizeOfUploadedData);
}
