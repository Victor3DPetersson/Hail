#include "Shared_PCH.h"
#include "Sorting.h"

#include "../Engine/RenderCommands.h"


using namespace Hail;


void Hail::Sorting::LinearBubbleDepthTypeCounter(DepthTypeCounter2D** pListToSort, uint32 listCapacity)
{
	for (int i = 0; i < listCapacity - 1; i++)
	{
		// Last i elements are already in place
		for (int j = 0; j < listCapacity - i - 1; j++)
		{
			// Comparing adjacent elements
			if ((*pListToSort)[j].m_layer > (*pListToSort)[j + 1].m_layer)
			{
				const DepthTypeCounter2D jVal = (*pListToSort)[j];
				(*pListToSort)[j] = (*pListToSort)[j + 1];
				(*pListToSort)[j + 1] = jVal;
			}
		}
	}
}

void Sorting::LinearBubbleSpriteCommand(GameCommand_Sprite** pListToSort, uint32 listCapacity)
{
	for (int i = 0; i < listCapacity - 1; i++)
	{
		// Last i elements are already in place
		for (int j = 0; j < listCapacity - i - 1; j++)
		{
			// Comparing adjacent elements
			const GameCommand_Sprite& jCommand = (*pListToSort)[j];
			const GameCommand_Sprite& jOneCommand = (*pListToSort)[j + 1];

			if (jCommand.m_layer >= jOneCommand.m_layer)
			{
				if (jCommand.materialInstanceID != jOneCommand.materialInstanceID || jCommand.index < jOneCommand.index)
				{
					const GameCommand_Sprite jVal = (*pListToSort)[j];
					(*pListToSort)[j] = (*pListToSort)[j + 1];
					(*pListToSort)[j + 1] = jVal;
				}
			}
		}
	}
}

void Sorting::LinearBubbleTextDepth(GameCommand_Text** pListToSort, uint32 listCapacity)
{
	for (int i = 0; i < listCapacity - 1; i++)
	{
		// Last i elements are already in place
		for (int j = 0; j < listCapacity - i - 1; j++)
		{
			// Comparing adjacent elements
			if ((*pListToSort)[j].m_layer > (*pListToSort)[j + 1].m_layer)
			{
				const GameCommand_Text jVal = (*pListToSort)[j];
				(*pListToSort)[j] = (*pListToSort)[j + 1];
				(*pListToSort)[j + 1] = jVal;
			}
		}
	}
}