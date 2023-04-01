#include "Engine_PCH.h"
#include "Renderer.h"
#include "DebugMacros.h"
#include "RenderCommands.h"

void Hail::Renderer::ReloadShaders()
{
	m_shadersRecompiled = true;
}

void Hail::Renderer::CalculateRenderResolution(glm::uvec2 windowResolution)
{
	m_windowResolution = windowResolution;
	const float aspectRatio16x9 = 16.0f / 9.0f;

	const float ratioX = float(m_windowResolution.x) / float(m_windowResolution.y);
	if (ratioX > aspectRatio16x9)
	{
		m_renderTargetResolution.x = m_windowResolution.y * aspectRatio16x9;
		m_renderTargetResolution.y = m_windowResolution.y;
	}
	else
	{
		m_renderTargetResolution.x = m_windowResolution.x;
		m_renderTargetResolution.y = m_windowResolution.x / aspectRatio16x9;
	}

	Debug_PrintConsoleString256(String256::Format("Window Res X: %i Window Res y: %i\nRender Target Res X: %i Render Target Res y: %i", m_windowResolution.x, m_windowResolution.y, m_renderTargetResolution.x, m_renderTargetResolution.y));
}

void Hail::Renderer::WindowSizeUpdated()
{
	m_framebufferResized = true;
}

void Hail::Renderer::StartFrame(RenderCommandPool& renderPool)
{
	//Move to general functions later
	const uint32_t numberOfSprites = renderPool.spriteCommands.Size();
	for (size_t sprite = 0; sprite < numberOfSprites; sprite++)
	{
		const RenderCommand_Sprite& spriteCommand = renderPool.spriteCommands[sprite];
		const glm::vec2 spriteScale = spriteCommand.transform.GetScale();
		const glm::vec2 spritePosition = spriteCommand.transform.GetPosition();
		SpriteInstanceData spriteInstance{};
		spriteInstance.position_scale = { spritePosition.x, spritePosition.y, spriteScale.x, spriteScale.y };
		spriteInstance.uvTR_BL = spriteCommand.uvTR_BL;
		spriteInstance.color = spriteCommand.color;
		spriteInstance.pivot_rotation_padding = { spriteCommand.pivot.x, spriteCommand.pivot.y, spriteCommand.transform.GetRotation() * Math::DegToRadf + Math::PIf * -0.5f, 0.0f };
		//Get sprite texture size and sort with material and everything here to the correct place. 
		spriteInstance.textureSize_effectData_padding = { 256, 256, static_cast<uint32_t>(spriteCommand.sizeRelativeToRenderTarget), 0 };
		m_spriteInstanceData.Add(spriteInstance);
	}
}

void Hail::Renderer::EndFrame()
{
	m_spriteInstanceData.Clear();
}
