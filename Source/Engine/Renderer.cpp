#include "Engine_PCH.h"
#include "Renderer.h"
#include "DebugMacros.h"
#include "RenderCommands.h"
#include "Resources\ResourceManager.h"
#include "Resources\MaterialManager.h"


void Hail::Renderer::WindowSizeUpdated()
{
	m_framebufferResized = true;
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
	BindMaterial(*m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::MODEL3D, 0), true);
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

	BindMaterial(*m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::DEBUG_LINES2D, 0), false);
	const uint32_t numberOfLines = m_commandPoolToRender->debugLineCommands.Size() * 2;
	RenderDebugLines2D(numberOfLines, 0);

	BindMaterial(*m_resourceManager->GetMaterialManager()->GetMaterial(eMaterialType::FULLSCREEN_PRESENT_LETTERBOX, 0), false);
	RenderLetterBoxPass();
}
