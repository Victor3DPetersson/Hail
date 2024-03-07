#include "Engine_PCH.h"
#include "InputActionMap.h"
#include "InputHandler.h"
#include "Containers\StaticArray\StaticArray.h"
#include "Utility\FilePath.hpp"
#include "Utility\InOutStream.h"
using namespace Hail;

namespace Local
{
	struct DefaultAction
	{
		InputActionMap::ActionCommon common;
		uint16 registeredInput;
	};
	GrowingArray<DefaultAction> GetDefaultInputMapping(InputHandler& inputHandler)
	{
		GrowingArray<DefaultAction> defaultInputMapping{
		{ eInputAction::PlayerMoveUp, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().W},
		{ eInputAction::PlayerMoveRight, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().D },
		{ eInputAction::PlayerMoveLeft, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().A },
		{ eInputAction::PlayerMoveDown, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().S },
		{ eInputAction::PlayerMoveUp, (uint8)InputActionMap::eInputType::GamepadButton, (uint8)eGamepadInputMapping::DPadUp },
		{ eInputAction::PlayerMoveRight, (uint8)InputActionMap::eInputType::GamepadButton, (uint8)eGamepadInputMapping::DPadRight  },
		{ eInputAction::PlayerMoveLeft, (uint8)InputActionMap::eInputType::GamepadButton, (uint8)eGamepadInputMapping::DPadLeft  },
		{ eInputAction::PlayerMoveDown, (uint8)InputActionMap::eInputType::GamepadButton, (uint8)eGamepadInputMapping::DPadDown  },
		{ eInputAction::PlayerMoveJoystickR, (uint8)InputActionMap::eInputType::Direction, (uint8)InputActionMap::eDirectionInputType::JoystickRight },
		{ eInputAction::PlayerMoveJoystickL, (uint8)InputActionMap::eInputType::Direction, (uint8)InputActionMap::eDirectionInputType::JoystickLeft },
		{ eInputAction::PlayerMoveJoystickL, (uint8)InputActionMap::eInputType::Direction, (uint8)InputActionMap::eDirectionInputType::Mouse},
		{ eInputAction::PlayerAction1, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().Q },
		{ eInputAction::PlayerAction2, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().E },
		{ eInputAction::PlayerPause, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().ESC },
		{ eInputAction::PlayerPause, (uint8)InputActionMap::eInputType::GamepadButton, (uint8)eGamepadInputMapping::Back },

		// Debug commands

		{ eInputAction::DebugAction1, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().F1 },
		{ eInputAction::DebugAction2, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().F2 },
		{ eInputAction::DebugAction3, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().F3 },
		{ eInputAction::DebugAction4, (uint8)InputActionMap::eInputType::Button, inputHandler.GetInputMapping().F4 }
		};
		return defaultInputMapping;
	}
		

}


void Hail::InputActionMap::Init(InputHandler* pInputHandler)
{
	m_pInputHandler = pInputHandler;
	FilePath userDirectory = FilePath::GetUserProjectDirectory();
	const FilePath finalPath = userDirectory + L"InputMap.inp";

	InOutStream inOutObject;
	if (finalPath.IsValid())
	{
		inOutObject.OpenFile(finalPath, FILE_OPEN_TYPE::READ, false);
		//TODO: Replace with a long string type
		GrowingArray<int8> characterStream(inOutObject.GetFileSize());
		characterStream.Fill();
		inOutObject.Read(characterStream.Data(), 1, inOutObject.GetFileSize());
		inOutObject.CloseFile();

		//Get lines
		GrowingArray<String256> lines(16);
		{
			String256 currentLine;
			uint32 currentCharCount = 0;
			for (size_t i = 0; i < characterStream.Size(); i++)
			{
				const int8 currentCharacter = characterStream[i];
				if (currentCharacter == '\n')
				{
					currentLine[currentCharCount] = 0;
					lines.Add(currentLine);
					currentLine[0] = 0;
					memset(currentLine.Data(), 0, 256);
					currentCharCount = 0;
					continue;
				}
				currentLine[currentCharCount] = currentCharacter;
				currentCharCount++;
			}
		}

		// Line to value 
		for (size_t i = 0; i < lines.Size(); i++)
		{
			String256& line = lines[i];
			int8 currentCharacter = line[0];
			uint32 currentCharacterCounter = 1;
			uint32 lastCommaPosition = 0;
			GrowingArray<uint32> values;
			while (currentCharacter != '\0')
			{
				currentCharacter = line[currentCharacterCounter++];
				if (currentCharacter == ',')
				{
					values.Add(StringUtility::IntFromConstChar(line.Data(), lastCommaPosition));
					lastCommaPosition = currentCharacterCounter;
				}
			}
			eInputAction currentActionType;
			eInputType currentInputType;

			for (size_t v = 0; v < values.Size(); v++)
			{

				if (v == 0)
				{
					currentActionType = (eInputAction)values[v];
					continue;
				}

				if (v == 1)
				{
					currentInputType = (eInputType)values[v];
					continue;
				}

				if (currentInputType == eInputType::Button)
					AddButtonInput(currentActionType, { (uint16)values[v] });
				else
				if (currentInputType == eInputType::Direction)
					AddDirectionInput(currentActionType, { (eDirectionInputType)values[v] });
				else
				if (currentInputType == eInputType::GamepadButton)
					AddGamepadButtonInput(currentActionType, { (eGamepadInputMapping)values[v] });
			}
		}

	}
	else
	{
		CreateDefaultInputMapping();
		SaveActionToInputMap();
	}
}

glm::vec2 Hail::InputActionMap::GetDirectionInput(eInputAction actionToGet, int gamepadToCheck) const
{

	for (size_t i = 0; i < m_directionActions.Size(); i++)
	{
		if (m_directionActions[i].actionData.action == actionToGet)
		{
			const DirectionAction& currentDirectionAction = m_directionActions[i];
			if (gamepadToCheck == 0)
			{
				if (!m_pInputHandler->IsGamepadActive(gamepadToCheck))
					return currentDirectionAction.currentDirections[0];
			}

			if (!m_pInputHandler->IsGamepadActive(gamepadToCheck))
				return Vec2Zero;


			if (currentDirectionAction.bRegisteredInputTypes[(uint8)eDirectionInputType::JoystickLeft])
			{
				return currentDirectionAction.currentDirections[gamepadToCheck];
			}
			if (currentDirectionAction.bRegisteredInputTypes[(uint8)eDirectionInputType::JoystickRight])
			{
				return currentDirectionAction.currentDirections[gamepadToCheck];
			}
		}
	}
	return Vec2Zero;
}

eInputState Hail::InputActionMap::GetButtonInput(eInputAction actionToGet, int gamepadToCheck) const
{
	for (size_t i = 0; i < m_buttonActions.Size(); i++)
	{
		if (m_buttonActions[i].actionData.action != actionToGet)
			continue;

		if (gamepadToCheck == 0)
		{
			if (!m_pInputHandler->IsGamepadActive(gamepadToCheck))
				return m_buttonActions[i].currentState;
		}
		break;
	}

 	for (size_t i = 0; i < m_gamepadButtonActions.Size(); i++)
	{
		const GamepadButtonAction& gamepadButtonAction = m_gamepadButtonActions[i];
		if (gamepadButtonAction.actionData.action != actionToGet)
			continue;

		return gamepadButtonAction.currentStates[gamepadToCheck];
	}

	return eInputState::None;
}

float Hail::InputActionMap::GetGamepadTriggerInput(eInputAction actionToGet, int gamepadToCheck) const
{
	for (size_t i = 0; i < m_gamepadTriggerActions.Size(); i++)
	{
		if (m_gamepadTriggerActions[i].actionData.action != actionToGet)
			continue;

		if (!m_pInputHandler->IsGamepadActive(gamepadToCheck))
			return 0.0f;

		for (size_t iTrigger = 0; iTrigger < 2; iTrigger++)
		{
			if (!m_gamepadTriggerActions[i].bRegisteredTriggers[iTrigger])
				continue;
			return m_gamepadTriggerActions[i].currentStates[gamepadToCheck];
		}
	}
	return 0.0f;
}

void Hail::InputActionMap::AddButtonInput(eInputAction actionToAdd, InputKey characterToBind)
{
	for (size_t i = 0; i < m_buttonActions.Size(); i++)
	{
		if (m_buttonActions[i].actionData.action == actionToAdd)
		{
			m_buttonActions[i].registeredKeys.Add(characterToBind);
			return;
		}
	}

	ButtonAction newAction;
	newAction.actionData.action = actionToAdd;
	newAction.actionData.type |= (uint8)eInputType::Button;
	newAction.registeredKeys.Add(characterToBind);
	m_buttonActions.Add(newAction);
}

void Hail::InputActionMap::AddGamepadButtonInput(eInputAction actionToAdd, eGamepadInputMapping buttonToBind)
{
	for (size_t i = 0; i < m_gamepadButtonActions.Size(); i++)
	{
		if (m_gamepadButtonActions[i].actionData.action == actionToAdd)
		{
			m_gamepadButtonActions[i].bRegisteredButtons[(uint16)buttonToBind] = true;
			return;
		}
	}

	GamepadButtonAction newAction;
	newAction.actionData.action = actionToAdd;
	newAction.actionData.type |= (uint8)eInputType::GamepadButton;
	memset(&newAction.currentStates, 0, sizeof(eInputState) * 4);
	memset(newAction.bRegisteredButtons.Data(), false, sizeof(bool) * (uint8)eGamepadInputMapping::Count);
	newAction.bRegisteredButtons[(uint16)buttonToBind] = true;
	m_gamepadButtonActions.Add(newAction);
}

void Hail::InputActionMap::AddGamepadTriggerInput(eInputAction actionToAdd, uint8 triggerToBind)
{
	if (triggerToBind > 1) 
	{
		//TODO: Add assert to this return statement
		return;
	}

	for (size_t i = 0; i < m_gamepadTriggerActions.Size(); i++)
	{
		if (m_gamepadTriggerActions[i].actionData.action == actionToAdd)
		{
			m_gamepadTriggerActions[i].bRegisteredTriggers[triggerToBind] = true;
			return;
		}
	}

	GamepadTriggerAction newAction{};
	newAction.actionData.action = actionToAdd;
	newAction.actionData.type |= (uint8)eInputType::GamepadTrigger;
	newAction.bRegisteredTriggers[0] = false;
	newAction.bRegisteredTriggers[1] = false;
	memset(newAction.currentStates.Data(), 0.0f, sizeof(float) * 4);
	m_gamepadTriggerActions.Add(newAction);
}

void Hail::InputActionMap::AddDirectionInput(eInputAction actionToAdd, eDirectionInputType inputToBind)
{
	for (size_t i = 0; i < m_directionActions.Size(); i++)
	{
		if (m_directionActions[i].actionData.action == actionToAdd)
		{
			m_directionActions[i].bRegisteredInputTypes[(uint32)inputToBind] = true;
			return;
		}
	}

	DirectionAction newAction{};
	newAction.actionData.action = actionToAdd;
	newAction.actionData.type |= (uint8)eInputType::Direction;
	memset(newAction.bRegisteredInputTypes, false, sizeof(bool) * (uint8)eDirectionInputType::Count);
	newAction.bRegisteredInputTypes[(uint32)inputToBind] = true;
	m_directionActions.Add(newAction);
}

void Hail::InputActionMap::UpdateInputActions()
{
	
	for (size_t iAction = 0; iAction < m_buttonActions.Size(); iAction++)
	{
		for (size_t iButton = 0; iButton < m_buttonActions[iAction].registeredKeys.Size(); iButton++)
		{
			const InputKey& currentKey = m_buttonActions[iAction].registeredKeys[iButton];

			const eInputState currentInputState = (eInputState)m_pInputHandler->GetInputMap().keyMap[currentKey.m_keyValue];

			// if only one registrered input, we can just use the registered input straight up.
			if (m_buttonActions[iAction].registeredKeys.Size() == 1)
			{
				m_buttonActions[iAction].currentState = currentInputState;
				break;
			}
			// But if there are more than one registered key, we take one that is not set to none, aka it is active. 
			// This can lead to if you have several buttons registered to the same action, the first button in the index list will set the input if pressed. (design choice)
			if (currentInputState != eInputState::None)
			{
				m_buttonActions[iAction].currentState = currentInputState;
				break;
			}
		}
	}
	for (size_t iGamepad = 0; iGamepad < 4; iGamepad++)
	{
		// If button is registered
		if (Gamepad* pGamepad = m_pInputHandler->GetGamePad(iGamepad))
		{
			for (size_t iAction = 0; iAction < m_gamepadButtonActions.Size(); iAction++)
			{
				GamepadButtonAction& currentAction = m_gamepadButtonActions[iAction];

				for (size_t iGamepadButton = 0; iGamepadButton < (uint8)eGamepadInputMapping::Count; iGamepadButton++)
				{
					if (currentAction.bRegisteredButtons[iGamepadButton])
					{
						const eInputState state = pGamepad->GetButtonInputState((eGamepadInputMapping)iGamepadButton);
						if (state != currentAction.currentStates[iGamepad])
							currentAction.currentStates[iGamepad] = state;
					}
 				}
			}

			for (size_t iAction = 0; iAction < m_gamepadTriggerActions.Size(); iAction++)
			{
				GamepadTriggerAction& currentAction = m_gamepadTriggerActions[iAction];

				for (size_t iTrigger = 0; iTrigger < 2; iTrigger++)
				{
					if (currentAction.bRegisteredTriggers[iTrigger])
					{
						currentAction.currentStates[iGamepad] = pGamepad->GetTriggerInputState(iTrigger);
					}
				}
			}
		}
		else
		{
			for (size_t iAction = 0; iAction < m_gamepadButtonActions.Size(); iAction++)
				m_gamepadButtonActions[iAction].currentStates[iGamepad] = eInputState::None;
		}
	}

	for (size_t iDAction = 0; iDAction < m_directionActions.Size(); iDAction++)
	{
		DirectionAction& directionAction = m_directionActions[iDAction];

		for (size_t iDirectionType = 0; iDirectionType < (uint32)eDirectionInputType::Count; iDirectionType++)
		{
			if (!directionAction.bRegisteredInputTypes[iDirectionType])
				continue;
			
			switch ((eDirectionInputType)iDirectionType)
			{
			case eDirectionInputType::Mouse:
				directionAction.currentDirections[0] = m_pInputHandler->GetInputMap().mouse.mouseDelta;
				break;
			case eDirectionInputType::MouseWheel:
				directionAction.currentDirections[0].x = m_pInputHandler->GetInputMap().mouse.scrollDelta;
				break;
			case eDirectionInputType::JoystickLeft:
				for (size_t iGamepad = 0; iGamepad < 4; iGamepad++)
				{
					if (Gamepad* pGamepad = m_pInputHandler->GetGamePad(iGamepad))
					{
						directionAction.currentDirections[iGamepad] = pGamepad->GetJoyStickDirection(0);
					}
					else
					{
						directionAction.currentDirections[iGamepad] = Vec2Zero;
					}
				}
				break;
			case eDirectionInputType::JoystickRight:
				for (size_t iGamepad = 0; iGamepad < 4; iGamepad++)
				{
					if (Gamepad* pGamepad = m_pInputHandler->GetGamePad(iGamepad))
					{
						directionAction.currentDirections[iGamepad] = pGamepad->GetJoyStickDirection(1);
					}
					else
					{
						directionAction.currentDirections[iGamepad] = Vec2Zero;
					}
				}
				break;
			case eDirectionInputType::Count:
				break;
			default:
				break;
			}
		}
	}
}

void Hail::InputActionMap::ResetInputMappingToDefault()
{
	// Reset the current action lists
	m_directionActions.RemoveAll();
	m_buttonActions.RemoveAll();
	m_gamepadButtonActions.RemoveAll();
	// Create new inputs with defaults
	CreateDefaultInputMapping();
}

InputMap Hail::InputActionMap::GetRawInputMap() const
{
	return m_pInputHandler->GetInputMap();
}

void Hail::InputActionMap::CreateDefaultInputMapping()
{
	const GrowingArray<Local::DefaultAction> defaultActions = Local::GetDefaultInputMapping(*m_pInputHandler);

	for (size_t i = 0; i < defaultActions.Size(); i++)
	{
		const Local::DefaultAction& action = defaultActions[i];
		if ((action.common.type & 0x1) == (uint8)eInputType::Button)
		{
			AddButtonInput(action.common.action, { action.registeredInput });
		}
		else if ((action.common.type & 0x2) == (uint8)eInputType::GamepadButton)
		{
			AddGamepadButtonInput(action.common.action, (eGamepadInputMapping)action.registeredInput);
		}
		else if ((action.common.type & 0x4) == (uint8)eInputType::GamepadTrigger)
		{
			AddGamepadTriggerInput(action.common.action, (uint8)action.registeredInput);
		}
		else if ((action.common.type & 0x10) == (uint8)eInputType::Direction)
		{
			AddDirectionInput(action.common.action, (eDirectionInputType)action.registeredInput);
		}
	}
}

void Hail::InputActionMap::SaveActionToInputMap() const
{
	FilePath userDirectory = FilePath::GetUserProjectDirectory();
	const FilePath finalPath = userDirectory + L"InputMap.inp";

	InOutStream inOutObject;
	inOutObject.OpenFile(finalPath, FILE_OPEN_TYPE::WRITE, false);

	const char comma = ',';
	const char newLine = '\n';

	
	for (size_t i = 0; i < m_buttonActions.Size(); i++)
	{
		const ButtonAction& action = m_buttonActions[i];
		String64 outAction;
		outAction = String64::Format("%i,%i,", action.actionData.action, action.actionData.type);
		inOutObject.Write(outAction.Data(), outAction.Length());

		for (size_t keys = 0; keys < action.registeredKeys.Size(); keys++)
		{
			String64 outKey;
			outKey = String64::Format("%i,", action.registeredKeys[keys]);
			inOutObject.Write(outKey.Data(), outKey.Length());
		}

		inOutObject.Write(&newLine, sizeof(char));
	}

	for (size_t i = 0; i < m_gamepadButtonActions.Size(); i++)
	{
		const GamepadButtonAction& action = m_gamepadButtonActions[i];
		String64 outAction;
		outAction = String64::Format("%i,%i,", action.actionData.action, action.actionData.type);
		inOutObject.Write(outAction.Data(), outAction.Length());

		for (size_t keys = 0; keys < (uint8)eGamepadInputMapping::Count; keys++)
		{
			if (!action.bRegisteredButtons[keys])
				continue;
			String64 outKey;
			outKey = String64::Format("%i,", keys);
			inOutObject.Write(outKey.Data(), outKey.Length());
		}

		inOutObject.Write(&newLine, sizeof(char));
	}

	for (size_t i = 0; i < m_gamepadTriggerActions.Size(); i++)
	{
		const GamepadTriggerAction& action = m_gamepadTriggerActions[i];
		String64 outAction;
		outAction = String64::Format("%i,%i,", action.actionData.action, action.actionData.type);
		inOutObject.Write(outAction.Data(), outAction.Length());

		for (size_t keys = 0; keys < 2; keys++)
		{
			if (!action.bRegisteredTriggers[keys])
				continue;
			String64 outKey;
			outKey = String64::Format("%i,", keys);
			inOutObject.Write(outKey.Data(), outKey.Length());
		}
		inOutObject.Write(&newLine, sizeof(char));
	}

	for (size_t i = 0; i < m_directionActions.Size(); i++)
	{
		const DirectionAction& action = m_directionActions[i];

		String64 outAction;
		outAction = String64::Format("%i,%i,", action.actionData.action, action.actionData.type);
		inOutObject.Write(outAction.Data(), outAction.Length());

		for (size_t keys = 0; keys < (uint32)eDirectionInputType::Count; keys++)
		{
			if (!action.bRegisteredInputTypes[keys])
				continue;
			String64 outKey;
			outKey = String64::Format("%i,", keys);
			inOutObject.Write(outKey.Data(), outKey.Length());
		}

		inOutObject.Write(&newLine, sizeof(char));
	}
}

Hail::InputActionMap::DirectionAction::DirectionAction() : 
	actionData()
{
	for (size_t i = 0; i < 4; i++)
	{
		currentDirections[i] = Vec2Zero;
	}
	memset(bRegisteredInputTypes, false, sizeof(bool) * (uint32)eDirectionInputType::Count);
}
