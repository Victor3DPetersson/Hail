#pragma once
#include "String.hpp"
#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\StaticArray\StaticArray.h"
#include "Containers\VectorOnStack\VectorOnStack.h"

namespace Hail
{
	enum class IMGUI_BEGIN_TYPES : uint32_t
	{
		WINDOW,
		DROPDOWN
	};

	constexpr uint32_t MAX_NUMBER_OF_IMGUI_RESPONSES = 1024;
	template<typename Type>
	class ImGuiCommand
	{
	public:
		Type GetResponseValue() { return m_responseVal; };
		Type& GetRequestValueRef() { return m_requestVal; };
		void SetValue(Type val);

		bool IsDirty() { return m_isDirty; }
		void TransferValue(ImGuiCommand<Type>& otherCommand);
		void TransferOneTimeValue();
	private:
		Type m_responseVal = Type();
		Type m_requestVal = Type();
		bool m_isDirty = false;
	};

	class ImGuiCommandRecorder
	{
	public:
		bool AddBeginCommand(const String256& windowName, uint32_t responseIndex);
		void AddCloseCommand();

		void AddCheckbox(const String256& name, uint32_t responseIndex, bool value);
		void SameLine();
		void Seperator();
		void AddFloatSlider(const String256& name, uint32_t responseIndex, float value);
		void AddIntSlider(const String256& name, uint32_t responseIndex, int value);
		bool AddButton(const String256& name, uint32_t responseIndex);
		bool AddTextInput(const String256& name, uint32_t responseIndex, const String256& textValue);
		template<typename Type>
		Type GetResponseValue(uint32_t responseIndex);

	private:
		void ClearAndTransferResponses(ImGuiCommandRecorder& transferFrom);
		friend class ImGuiCommandManager;
		StaticArray<ImGuiCommand<bool>, MAX_NUMBER_OF_IMGUI_RESPONSES> m_bools;
		StaticArray<ImGuiCommand<float>, MAX_NUMBER_OF_IMGUI_RESPONSES> m_floats;
		StaticArray<ImGuiCommand<int>, MAX_NUMBER_OF_IMGUI_RESPONSES> m_ints;
		StaticArray<ImGuiCommand<String256>, MAX_NUMBER_OF_IMGUI_RESPONSES> m_strings;
		enum class IMGUI_TYPES : uint32_t
		{
			WINDOW,
			DROPDOWN,
			SAME_LINE,
			SEPERATOR,
			CLOSEWINDOW,
			SLIDERF,
			SLIDERI,
			CHECKBOX,
			DROPDOWNWINDOW,
			BUTTON,
			TEXT_INPUT
		};

		struct ImGuiCommand
		{
			IMGUI_TYPES commandType;
			String256 name;
			uint32_t responseIndex;
		};
		GrowingArray<ImGuiCommand> m_commands;
	};

	class ImGuiCommandManager
	{
	public:
		void Init();
		void RenderImguiCommands();
		void SwitchCommandBuffers();
		//Figure out what to do with this later
		ImGuiCommandRecorder& FetchImguiResults() { return m_commandRecorder[m_writeCommandRecorder]; }
	private:

		uint32_t m_numberOfOpenWindows = 0;
		uint32_t m_readCommandRecorder = 0;
		uint32_t m_writeCommandRecorder = 1;
		ImGuiCommandRecorder m_commandRecorder[2];
	};

	template<typename Type>
	inline Type ImGuiCommandRecorder::GetResponseValue(uint32_t responseIndex)
	{
		return Type();
	}

	template<>
	inline float ImGuiCommandRecorder::GetResponseValue(uint32_t responseIndex)
	{
		return m_floats[responseIndex].GetResponseValue();
	}

	template<>
	inline int ImGuiCommandRecorder::GetResponseValue(uint32_t responseIndex)
	{
		return m_ints[responseIndex].GetResponseValue();
	}
	template<>
	inline bool ImGuiCommandRecorder::GetResponseValue(uint32_t responseIndex)
	{
		return m_bools[responseIndex].GetResponseValue();
	}

	template<>
	inline String256 ImGuiCommandRecorder::GetResponseValue(uint32_t responseIndex)
	{
		return m_strings[responseIndex].GetResponseValue();
	}

}
