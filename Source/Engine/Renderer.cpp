#include "Engine_PCH.h"
#include "Renderer.h"
#include "DebugMacros.h"
#include "RenderCommands.h"
#include "Resources\ResourceManager.h"
#include "Resources\MaterialManager.h"
#include "Rendering\FontRenderer.h"
#include "Rendering\CloudRenderer.h"
#include "Rendering\RenderContext.h"
#include "Resources\RenderingResourceManager.h"

Hail::Renderer::~Renderer()
{
	H_ASSERT(m_pFontRenderer == nullptr, "Never cleaned up the Renderer, Add the virtual Cleanup function to the inherited renderer")
}

bool Hail::Renderer::Initialize()
{
	bool initializationResult = true;
	m_pFontRenderer = new FontRenderer(this, m_pResourceManager);
	m_pCloudRenderer = new CloudRenderer(this, m_pResourceManager);
	initializationResult &= m_pFontRenderer->Initialize();
	initializationResult &= m_pCloudRenderer->Initialize();

	return initializationResult;
}

void Hail::Renderer::StartFrame(RenderCommandPool& renderPool)
{
	m_commandPoolToRender = &renderPool;
	m_pContext->StartFrame();
	m_pResourceManager->ReloadResources();
	m_pResourceManager->UpdateRenderBuffers(renderPool, m_pContext, m_timer);
	m_pFontRenderer->Prepare(renderPool);
	m_pCloudRenderer->Prepare(renderPool);
}

void Hail::Renderer::EndFrame()
{
	m_pResourceManager->ClearFrameData();
}

void Hail::Renderer::Render()
{
	m_pContext->StartGraphicsPass();
	m_pContext->TransferFramebufferLayout(m_pResourceManager->GetMainPassFBTexture(), eFrameBufferLayoutState::ColorAttachment, eFrameBufferLayoutState::DepthAttachment);
	m_pContext->BindFrameBufferAtSlot(m_pResourceManager->GetMainPassFBTexture(), 0);
	m_pContext->BindMaterial(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::MODEL3D, 0));

	m_pContext->ClearBoundFrameBuffers();
	if (!m_commandPoolToRender->m_meshData.Empty())
	{
		m_pContext->BindMaterial(m_pResourceManager->GetMaterialManager()->GetDefaultMaterial(eMaterialType::MODEL3D));
		m_pContext->BindVertexBuffer(m_pVertexBuffer, m_pIndexBuffer);
		RenderMesh(m_commandPoolToRender->m_meshData[0], 0);
	}

	//BindMaterial(*m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::DEBUG_LINES3D, 0), false);
	uint32 currentFontBatch = 0u;
	for (uint32 i = 0; i < m_commandPoolToRender->m_layersBatchOffset.Size(); i++)
	{
		uint32 nextOffset = i + 1 == m_commandPoolToRender->m_layersBatchOffset.Size() ? m_commandPoolToRender->m_batches.Size() : m_commandPoolToRender->m_layersBatchOffset[i + 1];
		uint32 numberOfBatches = nextOffset - m_commandPoolToRender->m_layersBatchOffset[i];
		for (uint32 iBatch = 0; iBatch < numberOfBatches; iBatch++)
		{
			const Batch2DInfo& batchToRender = m_commandPoolToRender->m_batches[m_commandPoolToRender->m_layersBatchOffset[i] + iBatch];
			if (batchToRender.m_type == eCommandType::Sprite)
			{
				RenderCommand2DBase& firstCommand = m_commandPoolToRender->m_2DRenderCommands[batchToRender.m_instanceOffset];
				H_ASSERT(firstCommand.m_index_materialIndex_flags.u & IsSpriteFlagMask, "Invalid RenderCommand");
				const uint32 flagsToRemove = (LerpCommandFlagMask | IsSpriteFlagMask) >> 16;
				const uint32 materialUnpackedIndex = (firstCommand.m_index_materialIndex_flags.u >> 16) & (~flagsToRemove);
				const bool validMaterial = materialUnpackedIndex != MaterialIndexIsValidMask;
				const uint32 materialInstanceIndex = validMaterial ? materialUnpackedIndex : MAX_UINT;
				const MaterialInstance& materialInstance = m_pResourceManager->GetMaterialManager()->GetMaterialInstance(materialInstanceIndex, eMaterialType::SPRITE);
				m_pContext->BindMaterial(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::SPRITE, materialInstance.m_materialIndex));
				m_pContext->BindMaterialInstance(materialInstance.m_gpuResourceInstance);
				m_pContext->BindVertexBuffer(m_pSpriteVertexBuffer, nullptr);
				m_pContext->RenderSprites(batchToRender.m_numberOfInstances, batchToRender.m_instanceOffset);
			}
			else
			{
				RenderCommand2DBase& firstCommand = m_commandPoolToRender->m_2DRenderCommands[batchToRender.m_instanceOffset];
				H_ASSERT((firstCommand.m_index_materialIndex_flags.u & IsSpriteFlagMask) == false, "Invalid RenderCommand");
				m_pFontRenderer->RenderBatch(batchToRender.m_numberOfInstances, currentFontBatch);
				currentFontBatch++;
			}
		}
	}

	m_pCloudRenderer->Render();

	const uint32_t numberOfLines = m_commandPoolToRender->m_debugLineCommands.Size() * 2;
	if (numberOfLines)
	{
		m_pContext->BindMaterial(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::DEBUG_LINES2D, 0));
		m_pContext->BindVertexBuffer(m_pDebugLineVertexBuffer, nullptr);
		RenderDebugLines2D(numberOfLines, 0);
	}

	// Finished rendering to our main framebuffer
	m_pContext->EndRenderPass();

	m_pContext->TransferFramebufferLayout(m_pResourceManager->GetMainPassFBTexture(), eFrameBufferLayoutState::ShaderRead, eFrameBufferLayoutState::ShaderRead);
	m_pContext->BindFrameBufferAtSlot(m_pResourceManager->GetSwapChain()->GetFrameBufferTexture(), 0);
	m_pContext->BindMaterial(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::FULLSCREEN_PRESENT_LETTERBOX, 0));
	m_pContext->RenderFullscreenPass();
	RenderImGui();
	m_pContext->EndGraphicsPass();
}

void Hail::Renderer::Cleanup()
{
	H_ASSERT(m_renderDevice, "Base function must be called before cleaning up the child.")
	m_pFontRenderer->Cleanup();
	SAFEDELETE(m_pFontRenderer);
	m_pCloudRenderer->Cleanup();
	SAFEDELETE(m_pCloudRenderer);
}


void Hail::Renderer::CreateSpriteVertexBuffer()
{
	uint32_t vertices[6] = { 0, 1, 2, 3, 4, 5 };

	BufferProperties spriteVertexBufferProperties;
	spriteVertexBufferProperties.elementByteSize = sizeof(uint32_t);
	spriteVertexBufferProperties.numberOfElements = 6;
	spriteVertexBufferProperties.offset = 0;
	spriteVertexBufferProperties.type = eBufferType::vertex;
	spriteVertexBufferProperties.domain = eShaderBufferDomain::GpuOnly;
	spriteVertexBufferProperties.usage = eShaderBufferUsage::Read;
	spriteVertexBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
	m_pSpriteVertexBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(spriteVertexBufferProperties);

	m_pContext->UploadDataToBuffer(m_pSpriteVertexBuffer, vertices, sizeof(uint32_t) * 6);
}

void Hail::Renderer::CreateVertexBuffer()
{
	BufferProperties vertexBufferProperties;
	vertexBufferProperties.elementByteSize = sizeof(VertexModel);
	vertexBufferProperties.numberOfElements = m_pResourceManager->m_unitCube.vertices.Size();
	vertexBufferProperties.offset = 0;
	vertexBufferProperties.type = eBufferType::vertex;
	vertexBufferProperties.domain = eShaderBufferDomain::GpuOnly;
	vertexBufferProperties.usage = eShaderBufferUsage::Read;
	vertexBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
	m_pVertexBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(vertexBufferProperties);

	m_pContext->UploadDataToBuffer(m_pVertexBuffer, m_pResourceManager->m_unitCube.vertices.Data(), sizeof(VertexModel) * m_pResourceManager->m_unitCube.vertices.Size());
}

void Hail::Renderer::CreateIndexBuffer()
{
	BufferProperties indexBufferProperties;
	indexBufferProperties.elementByteSize = sizeof(uint32_t);
	indexBufferProperties.numberOfElements = m_pResourceManager->m_unitCube.indices.Size();
	indexBufferProperties.offset = 0;
	indexBufferProperties.type = eBufferType::index;
	indexBufferProperties.domain = eShaderBufferDomain::GpuOnly;
	indexBufferProperties.usage = eShaderBufferUsage::Read;
	indexBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
	m_pIndexBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(indexBufferProperties);
	m_pContext->UploadDataToBuffer(m_pIndexBuffer, m_pResourceManager->m_unitCube.indices.Data(), sizeof(uint32_t) * m_pResourceManager->m_unitCube.indices.Size());
}

void Hail::Renderer::CreateDebugLineVertexBuffer()
{
	GrowingArray<uint32> debugLineVertices(MAX_NUMBER_OF_DEBUG_LINES, 0);
	for (uint32 i = 0; i < MAX_NUMBER_OF_DEBUG_LINES; i++)
	{
		debugLineVertices[i] = i;
	}

	BufferProperties debugLineBufferProperties;
	debugLineBufferProperties.elementByteSize = sizeof(uint32_t);
	debugLineBufferProperties.numberOfElements = MAX_NUMBER_OF_DEBUG_LINES;
	debugLineBufferProperties.offset = 0;
	debugLineBufferProperties.type = eBufferType::vertex;
	debugLineBufferProperties.domain = eShaderBufferDomain::GpuOnly;
	debugLineBufferProperties.usage = eShaderBufferUsage::Read;
	debugLineBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
	m_pDebugLineVertexBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(debugLineBufferProperties);
	m_pContext->UploadDataToBuffer(m_pDebugLineVertexBuffer, debugLineVertices.Data(), sizeof(uint32_t) * MAX_NUMBER_OF_DEBUG_LINES);
}
