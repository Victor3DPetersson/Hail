#include "Engine_PCH.h"
#include "ImGuiCommands.h"
#include "imgui.h"

bool Hail::ImGuiCommandRecorder::AddBeginCommand(const String256& windowName, uint32_t responseIndex)
{
	ImGuiCommand command = { IMGUI_TYPES::WINDOW, windowName, responseIndex };
	m_commands.Add(command);
	return m_bools[responseIndex].GetResponseValue();
}

void Hail::ImGuiCommandRecorder::AddCloseCommand()
{
	ImGuiCommand command = { IMGUI_TYPES::CLOSEWINDOW, ""};
	m_commands.Add(command);
}

void Hail::ImGuiCommandRecorder::SameLine()
{
	ImGuiCommand command = { IMGUI_TYPES::SAME_LINE, "" };
	m_commands.Add(command);
}

void Hail::ImGuiCommandRecorder::Seperator()
{
	ImGuiCommand command = { IMGUI_TYPES::SEPERATOR, "" };
	m_commands.Add(command);
}

void Hail::ImGuiCommandRecorder::AddCheckbox(const String256& name, uint32_t responseIndex, bool value)
{
	ImGuiCommand command = { IMGUI_TYPES::CHECKBOX, name, responseIndex };
	m_commands.Add(command);
	m_bools[responseIndex].SetValue(value);
}

void Hail::ImGuiCommandRecorder::AddFloatSlider(const String256& name, uint32_t responseIndex, float value)
{
	ImGuiCommand command = { IMGUI_TYPES::SLIDERF, name, responseIndex };
	m_commands.Add(command);
	m_floats[responseIndex].SetValue(value);
}

void Hail::ImGuiCommandRecorder::AddIntSlider(const String256& name, uint32_t responseIndex, int value)
{
	ImGuiCommand command = { IMGUI_TYPES::SLIDERI, name, responseIndex };
	m_commands.Add(command);
	m_ints[responseIndex].SetValue(value);
}

bool Hail::ImGuiCommandRecorder::AddButton(const String256& name, uint32_t responseIndex)
{
	ImGuiCommand command = { IMGUI_TYPES::BUTTON, name, responseIndex };
	m_commands.Add(command);
	return m_bools[responseIndex].GetResponseValue();
}

bool Hail::ImGuiCommandRecorder::AddTextInput(const String256& name, uint32_t responseIndex, const String256& textValue)
{
	ImGuiCommand command = { IMGUI_TYPES::TEXT_INPUT, name, responseIndex };
	m_commands.Add(command);
	m_strings[responseIndex].SetValue(textValue);
	return m_bools[responseIndex].GetResponseValue();
}

void Hail::ImGuiCommandRecorder::ClearAndTransferResponses(ImGuiCommandRecorder& transferFrom)
{
	const uint32_t commandsSize = m_commands.Size();
	for (size_t i = 0; i < commandsSize; i++)
	{
		ImGuiCommandRecorder::ImGuiCommand& command = m_commands[i];
		switch (command.commandType)
		{
		case ImGuiCommandRecorder::IMGUI_TYPES::WINDOW:
			m_bools[command.responseIndex].TransferOneTimeValue();
			m_bools[command.responseIndex].GetRequestValueRef() = false;
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::DROPDOWN:

			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::CLOSEWINDOW:
			break;

		case ImGuiCommandRecorder::IMGUI_TYPES::SLIDERF:
			m_floats[command.responseIndex].TransferValue(transferFrom.m_floats[command.responseIndex]);
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::SLIDERI:
			m_ints[command.responseIndex].TransferValue(transferFrom.m_ints[command.responseIndex]);

			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::CHECKBOX:
			m_bools[command.responseIndex].TransferValue(transferFrom.m_bools[command.responseIndex]);
			break;

		case ImGuiCommandRecorder::IMGUI_TYPES::BUTTON:
			m_bools[command.responseIndex].TransferOneTimeValue();
			m_bools[command.responseIndex].GetRequestValueRef() = false;
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::TEXT_INPUT:
			m_bools[command.responseIndex].TransferOneTimeValue();
			m_bools[command.responseIndex].GetRequestValueRef() = false;
			m_strings[command.responseIndex].TransferValue(transferFrom.m_strings[command.responseIndex]);
			break;
		default:
			break;
		}
	}
	m_commands.RemoveAll();
}

void Hail::ImGuiCommandManager::Init()
{
	m_commandRecorder[0].m_commands.Init(100);
	m_commandRecorder[1].m_commands.Init(100);
}

void Hail::ImGuiCommandManager::RenderImguiCommands()
{

	ImGuiCommandRecorder& recorder = m_commandRecorder[m_readCommandRecorder];
	const uint32_t commandsSize = recorder.m_commands.Size();
	bool testBoolList[5];
	for (size_t i = 0; i < commandsSize; i++)
	{
		ImGuiCommandRecorder::ImGuiCommand& command = recorder.m_commands[i];
		switch (command.commandType)
		{
		case ImGuiCommandRecorder::IMGUI_TYPES::WINDOW:
			m_numberOfOpenWindows++;
			if (ImGui::Begin(command.name.Data()))
			{
				recorder.m_bools[command.responseIndex].GetRequestValueRef() = true;
			}
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::DROPDOWN:
			//if (ImGui::BeginCombo("combo 1", "Din Mamma"))
			//{
			//	ImGui::Selectable("iS THIS SELECTABLE?", testBoolList, ImGuiSelectableFlags_::ImGuiSelectableFlags_AllowDoubleClick);
			//	ImGui::EndCombo();
			//}
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::SAME_LINE:
			ImGui::SameLine();
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::SEPERATOR:
			ImGui::Separator();
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::CLOSEWINDOW:
			if (m_numberOfOpenWindows != 0)
			{

				m_numberOfOpenWindows--;
				ImGui::End();
			}
			//else error handling
			break;

		case ImGuiCommandRecorder::IMGUI_TYPES::SLIDERF:
			if (m_numberOfOpenWindows != 0)
			{
				ImGui::DragFloat(command.name.Data(), &recorder.m_floats[command.responseIndex].GetRequestValueRef());
			}
			//else error handling
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::SLIDERI:
			if (m_numberOfOpenWindows != 0)
			{
				ImGui::DragInt(command.name.Data(), &recorder.m_ints[command.responseIndex].GetRequestValueRef());
			}
			//else error handling
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::CHECKBOX:
			if (m_numberOfOpenWindows != 0)
			{
				ImGui::Checkbox(command.name.Data(), &recorder.m_bools[command.responseIndex].GetRequestValueRef());
			}
			//else error handling
			break;

		case ImGuiCommandRecorder::IMGUI_TYPES::BUTTON:
			if (m_numberOfOpenWindows != 0)
			{
				if (ImGui::Button(command.name.Data()))
				{
					recorder.m_bools[command.responseIndex].GetRequestValueRef() = true;
				}
			}
			//else error handling
			break;
		case ImGuiCommandRecorder::IMGUI_TYPES::TEXT_INPUT:
			if (m_numberOfOpenWindows != 0)
			{
				if (ImGui::InputText(command.name.Data(), recorder.m_strings[command.responseIndex].GetRequestValueRef().Data(), 256, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
				{
					recorder.m_bools[command.responseIndex].GetRequestValueRef() = true;
				}
			}
			//else error handling
			break;
		default:
			break;
		}
	}

	if (m_numberOfOpenWindows != 0)
	{
		for (size_t i = 0; i < m_numberOfOpenWindows; i++)
		{
			m_numberOfOpenWindows--;
			ImGui::End();
		}
	}
	//else error handling
}

void Hail::ImGuiCommandManager::SwitchCommandBuffers()
{
	//Read will become write, transfer read to writes values
	m_commandRecorder[m_readCommandRecorder].ClearAndTransferResponses(m_commandRecorder[m_writeCommandRecorder]);
	const uint32_t oldWrite = m_writeCommandRecorder;
	m_writeCommandRecorder = m_readCommandRecorder;
	m_readCommandRecorder = oldWrite;
}

template<typename Type>
inline void Hail::ImGuiCommand<Type>::SetValue(Type val)
{
	m_requestVal = val;
	if (val != m_responseVal)
	{
		m_isDirty = true;
	}
}

template<typename Type>
inline void Hail::ImGuiCommand<Type>::TransferValue(ImGuiCommand<Type>& otherCommand)
{
	m_responseVal = m_requestVal;
	if (otherCommand.m_isDirty)
	{
		m_responseVal = otherCommand.m_requestVal;
	}
	else
	{
		otherCommand.m_requestVal = m_responseVal;
	}
	otherCommand.m_isDirty = false;
}

template<typename Type>
inline void Hail::ImGuiCommand<Type>::TransferOneTimeValue()
{
	m_responseVal = m_requestVal;
}