#include "Engine_PCH.h"
#include "Renderer.h"
#include "DebugMacros.h"
#include "RenderCommands.h"
#include "Resources\ResourceManager.h"
#include "Resources\MaterialManager.h"
#include "Rendering\FontRenderer.h"
#include "Rendering\RenderContext.h"

void Hail::Renderer::WindowSizeUpdated()
{
	m_framebufferResized = true;
}

bool Hail::Renderer::Initialize()
{
	m_pContext = new RenderContext(m_resourceManager);

	bool initializationResult = true;
	m_pFontRenderer = new FontRenderer(this, m_resourceManager);
	initializationResult &= m_pFontRenderer->Initialize();

	return initializationResult;
}

void Hail::Renderer::StartFrame(RenderCommandPool& renderPool)
{
	m_commandPoolToRender = &renderPool;
	m_resourceManager->ReloadResources();
	m_resourceManager->UpdateRenderBuffers(renderPool, m_timer);
}

void Hail::Renderer::EndFrame()
{
	m_resourceManager->ClearFrameData();
}

void Hail::Renderer::Render()
{
	BindMaterialPipeline(m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::MODEL3D, 0)->m_pPipeline, true);
	if (!m_commandPoolToRender->meshCommands.Empty())
	{
		RenderMesh(m_commandPoolToRender->meshCommands[0], 0);
	}

	//BindMaterial(*m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::DEBUG_LINES3D, 0), false);

	const uint32_t numberOfSprites = m_commandPoolToRender->spriteCommands.Size();
	for (size_t sprite = 0; sprite < numberOfSprites; sprite++)
	{
		RenderSprite(m_commandPoolToRender->spriteCommands[sprite], sprite);
	}

	BindMaterialPipeline(m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::DEBUG_LINES2D, 0)->m_pPipeline, false);
	const uint32_t numberOfLines = m_commandPoolToRender->debugLineCommands.Size() * 2;
	RenderDebugLines2D(numberOfLines, 0);

	m_pFontRenderer->Render();

	BindMaterialPipeline(m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::FULLSCREEN_PRESENT_LETTERBOX, 0)->m_pPipeline, false);
	RenderLetterBoxPass();
}
