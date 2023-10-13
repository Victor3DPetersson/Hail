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
	m_resourceManager->UpdateRenderBuffers(renderPool, m_timer);
}

void Hail::Renderer::EndFrame()
{
	m_resourceManager->ClearFrameData();
}

void Hail::Renderer::Render()
{
	BindMaterial(m_resourceManager->GetMaterialManager()->GetMaterial(MATERIAL_TYPE::MODEL3D));
	if (!m_commandPoolToRender->meshCommands.Empty())
	{
		RenderMesh(m_commandPoolToRender->meshCommands[0], 0);
	}
	EndMaterialPass();

	BindMaterial(m_resourceManager->GetMaterialManager()->GetMaterial(MATERIAL_TYPE::SPRITE));
	const uint32_t numberOfSprites = m_commandPoolToRender->spriteCommands.Size();
	for (size_t sprite = 0; sprite < numberOfSprites; sprite++)
	{
		RenderSprite(m_commandPoolToRender->spriteCommands[sprite], sprite);
	}
	EndMaterialPass();

	BindMaterial(m_resourceManager->GetMaterialManager()->GetMaterial(MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX));
	RenderLetterBoxPass();
}
