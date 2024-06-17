#pragma once
#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\StaticArray\StaticArray.h"
namespace Hail
{
	struct ErrorMessage;
	class ImGuiMessageLogger
	{
	public:
		ImGuiMessageLogger();

		void RenderImGuiCommands();

	private:
		enum class eMessageSortingType : uint8
		{
			MessageType,
			Message,
			NumberOfOccurences,
			Time,
			File,
			Count
		};
		void FillAndSortMessageList();

		eMessageSortingType m_currentSortingType;
		StaticArray<bool, (uint8)eMessageType::Count> m_visibleTypes;
		GrowingArray<const ErrorMessage*> m_sortedMessages;
	};
}


