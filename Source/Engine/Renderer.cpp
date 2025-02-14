#include "Engine_PCH.h"
#include "Renderer.h"
#include "DebugMacros.h"
#include "RenderCommands.h"
#include "Resources\ResourceManager.h"
#include "Resources\MaterialManager.h"
#include "Rendering\FontRenderer.h"
#include "Rendering\RenderContext.h"

Hail::Renderer::~Renderer()
{
	H_ASSERT(m_pFontRenderer == nullptr, "Never cleaned up the Renderer, Add the virtual Cleanup function to the inherited renderer")
}

bool Hail::Renderer::Initialize()
{
	bool initializationResult = true;
	m_pFontRenderer = new FontRenderer(this, m_pResourceManager);
	initializationResult &= m_pFontRenderer->Initialize();

	return initializationResult;
}

void Hail::Renderer::StartFrame(RenderCommandPool& renderPool)
{
	m_commandPoolToRender = &renderPool;
	m_pContext->StartFrame();
	m_pResourceManager->ReloadResources();
	m_pResourceManager->UpdateRenderBuffers(renderPool, m_pContext, m_timer);
	m_pFontRenderer->Prepare(renderPool);
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
	//BindMaterialPipeline(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::MODEL3D, 0)->m_pPipeline, true);
	if (!m_commandPoolToRender->meshCommands.Empty())
	{
		RenderMesh(m_commandPoolToRender->meshCommands[0], 0);
	}

	//BindMaterial(*m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::DEBUG_LINES3D, 0), false);

	const uint32_t numberOfSprites = m_commandPoolToRender->spriteCommands.Size();
	for (size_t sprite = 0; sprite < numberOfSprites; sprite++)
	{
		const MaterialInstance& materialInstance = m_pResourceManager->GetMaterialManager()->GetMaterialInstance(m_commandPoolToRender->spriteCommands[sprite].materialInstanceID, eMaterialType::SPRITE);
		m_pContext->BindMaterial(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::SPRITE, materialInstance.m_materialIndex));

		RenderSprite(m_commandPoolToRender->spriteCommands[sprite], sprite);
	}

	//BindMaterialPipeline(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::DEBUG_LINES2D, 0)->m_pPipeline, false);
	const uint32_t numberOfLines = m_commandPoolToRender->debugLineCommands.Size() * 2;
	if (numberOfLines)
	{
		m_pContext->BindMaterial(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::DEBUG_LINES2D, 0));
		RenderDebugLines2D(numberOfLines, 0);
	}
	m_pFontRenderer->Render();

	// Finished rendering to our main framebuffer
	m_pContext->EndRenderPass();

	m_pContext->TransferFramebufferLayout(m_pResourceManager->GetMainPassFBTexture(), eFrameBufferLayoutState::ShaderRead, eFrameBufferLayoutState::ShaderRead);
	m_pContext->BindFrameBufferAtSlot(m_pResourceManager->GetSwapChain()->GetFrameBufferTexture(), 0);
	m_pContext->BindMaterial(m_pResourceManager->GetMaterialManager()->GetMaterial(eMaterialType::FULLSCREEN_PRESENT_LETTERBOX, 0));
	// TODO: flytta till contexten
	RenderLetterBoxPass();
	m_pContext->EndGraphicsPass();
}

void Hail::Renderer::Cleanup()
{
	H_ASSERT(m_renderDevice, "Base function must be called before cleaning up the child.")
	m_pFontRenderer->Cleanup();
	SAFEDELETE(m_pFontRenderer);
}
