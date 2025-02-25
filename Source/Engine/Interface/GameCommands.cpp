#include "Engine_PCH.h"
#include "GameCommands.h"

void Hail::ApplicationCommandPool::AddDebugLine(const DebugLineCommand& debugLineToAdd)
{
	m_debugLineCommands.Add(debugLineToAdd);
}

void Hail::ApplicationCommandPool::AddSpriteCommand(const GameCommand_Sprite& spriteToAdd)
{
	m_spriteCommands.Add(spriteToAdd);
	for (uint32 i = 0; i < m_depthTypeCounters.Size(); i++)
	{
		if (spriteToAdd.m_layer == m_depthTypeCounters[i].m_layer)
		{
			m_depthTypeCounters[i].m_spriteCounter++;
			return;
		}
	}
	DepthTypeCounter2D& depthTypeCounter = m_depthTypeCounters.Add();
	depthTypeCounter.m_layer = spriteToAdd.m_layer;
	depthTypeCounter.m_spriteCounter = 1;
	depthTypeCounter.m_textCounter = 0;
}

void Hail::ApplicationCommandPool::AddTextCommand(const GameCommand_Text& textToAdd)
{
	m_textCommands.Add(textToAdd);
	for (uint32 i = 0; i < m_depthTypeCounters.Size(); i++)
	{
		if (textToAdd.m_layer == m_depthTypeCounters[i].m_layer)
		{
			m_depthTypeCounters[i].m_textCounter++;
			return;
		}
	}
	DepthTypeCounter2D& depthTypeCounter = m_depthTypeCounters.Add();
	depthTypeCounter.m_layer = textToAdd.m_layer;
	depthTypeCounter.m_spriteCounter = 0;
	depthTypeCounter.m_textCounter = 1;
}

void Hail::ApplicationCommandPool::NewFrame()
{
	m_depthTypeCounters.Clear();
	m_debugLineCommands.Clear();
	m_spriteCommands.Clear();
	m_textCommands.Clear();
	m_meshCommands.Clear();
}
