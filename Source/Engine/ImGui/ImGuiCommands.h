#pragma once
#include "String.hpp"
#include "Types.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\StaticArray\StaticArray.h"
#include "Containers\VectorOnStack\VectorOnStack.h"
#include "ImGuiFileBrowser.h"
#include "Utility\FilePath.hpp"

namespace Hail
{
	class ResourceManager;
	class RenderContext;

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
		bool AddBeginCommand(const String64& windowName, uint32_t responseIndex);
		void AddCloseCommand();

		void AddCheckbox(const String64& name, uint32_t responseIndex, bool value);
		void AddSameLine();
		void AddSeperator();

		bool AddTreeNode(const String64& name, uint32_t responseIndex);
		void AddTreeNodeEnd();
		bool AddTabBar(const String64& name, uint32_t responseIndex);
		void AddTabBarEnd();
		bool AddTabItem(const String64& name, uint32_t responseIndex);
		void AddTabItemEnd();

		void AddFloatSlider(const String64& name, uint32_t responseIndex, float value);
		void AddIntSlider(const String64& name, uint32_t responseIndex, int value);
		bool AddButton(const String64& name, uint32_t responseIndex);
		bool AddTextInput(const String64& name, uint32_t responseIndex, const StringL& textValue);
		void OpenFileBrowser(ImGuiFileBrowserData* fileBrowserDataToFill);
		void OpenMaterialEditor();
		template<typename Type>
		Type GetResponseValue(uint32_t responseIndex);

	private:
		void ClearAndTransferResponses(ImGuiCommandRecorder& writeFrom);
		friend class ImGuiCommandManager;
		StaticArray<ImGuiCommand<bool>, MAX_NUMBER_OF_IMGUI_RESPONSES> m_bools;
		StaticArray<ImGuiCommand<float>, MAX_NUMBER_OF_IMGUI_RESPONSES> m_floats;
		StaticArray<ImGuiCommand<int>, MAX_NUMBER_OF_IMGUI_RESPONSES> m_ints;
		StaticArray<ImGuiCommand<StringL>, MAX_NUMBER_OF_IMGUI_RESPONSES> m_strings;
		enum class IMGUI_TYPES : uint32_t
		{
			WINDOW,
			CLOSEWINDOW,
			BEGIN_TREENODE,
			END_TREENODE,
			BEGIN_TAB_BAR,
			END_TAB_BAR,
			BEGIN_TAB_ITEM,
			END_TAB_ITEM,
			DROPDOWN,
			SAME_LINE,
			SEPERATOR,
			SLIDERF,
			SLIDERI,
			CHECKBOX,
			DROPDOWNWINDOW,
			BUTTON,
			TEXT_INPUT,
			FILE_BROWSER,
			//TODO: remove this type as the game code should not handle assets in this way
			MATERIAL_EDITOR,
			COUNT
		};

		struct ImGuiCommand
		{
			IMGUI_TYPES commandType;
			String64 name;
			uint32_t responseIndex;
		};
		GrowingArray<ImGuiCommand> m_commands;
		bool m_lockGameThread = false;
		void* m_lockThreadData = nullptr;
		IMGUI_TYPES m_lockType = IMGUI_TYPES::COUNT;
	};

	class ImGuiCommandManager
	{
	public:
		void Init(ResourceManager* resourceManager);
		void DeInit();
		void RenderImguiCommands(RenderContext* pRenderContext);
		void RenderSingleImguiCommand(bool& unlockApplicationThread, RenderContext* pRenderContext);
		void SwitchCommandBuffers(bool& shouldLockApplicationThread);
		//Figure out what to do with this later
		ImGuiCommandRecorder& FetchImguiResults() { return m_commandRecorder[m_writeCommandRecorder]; }
	private:
		void RenderErrorModal(bool& unlockApplicationThread);
		void PopLatestStackType();
		void PopDownToType(ImGuiCommandRecorder::IMGUI_TYPES typeToPop);
		void PopStackType(ImGuiCommandRecorder::IMGUI_TYPES referenceTypeToPop);
		void SendImGuiPopCommand(ImGuiCommandRecorder::IMGUI_TYPES typeToPop);

		void RenderEngineImgui(RenderContext* pRenderContext);

		uint32 m_numberOfOpenWindows = 0;
		uint32 m_numberOfOpenTabItems = 0;
		uint32 m_numberOfOpenTabBars = 0;
		uint32 m_numberOfOpenTreeNodes = 0;
		uint32 m_readCommandRecorder = 0;
		uint32 m_writeCommandRecorder = 1;

		VectorOnStack<ImGuiCommandRecorder::IMGUI_TYPES, 128> m_pushedTypeStack;
		void* m_lockThreadData = nullptr;
		bool m_successfullySetupSingleRenderSystem = false;
		ImGuiCommandRecorder::IMGUI_TYPES m_singleCommandRenderType = ImGuiCommandRecorder::IMGUI_TYPES::COUNT;
		ImGuiFileBrowser m_fileBrowser;

		ImGuiCommandRecorder m_commandRecorder[2];
		ResourceManager* m_resourceManager = nullptr;

		// TODO, have a more robust setting for the imgui code and serialize this value
		bool m_openAssetBrowser = false;
		bool m_openPropertyWindow = false;
		bool m_openMessageLogger = false;
		bool m_openMaterialWindow = false;
		bool m_bOpenProfilerlWindow = true;
	};

	template<typename Type>
	inline Type ImGuiCommandRecorder::GetResponseValue(uint32 responseIndex)
	{
		return Type();
	}

	template<>
	inline float ImGuiCommandRecorder::GetResponseValue(uint32 responseIndex)
	{
		return m_floats[responseIndex].GetResponseValue();
	}

	template<>
	inline int ImGuiCommandRecorder::GetResponseValue(uint32 responseIndex)
	{
		return m_ints[responseIndex].GetResponseValue();
	}
	template<>
	inline bool ImGuiCommandRecorder::GetResponseValue(uint32 responseIndex)
	{
		return m_bools[responseIndex].GetResponseValue();
	}

	template<>
	inline StringL& ImGuiCommandRecorder::GetResponseValue(uint32 responseIndex)
	{
		return m_strings[responseIndex].GetResponseValue();
	}

}
