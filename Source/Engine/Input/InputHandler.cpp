#include "Engine_PCH.h"
#include "InputHandler.h"
#include "MathUtils.h"

using namespace Hail;

InputHandler::InputHandler() :
	m_cursorLock(false),
	m_inputIsPaused(false),
	m_mainInputIsGamepad(false)
{
	for (size_t i = 0; i < 4; i++)
	{
		m_pGamepads[i] = nullptr;
	}
}

void InputHandler::UpdateKeyStates()
{
	for (size_t i = 0; i < 0xff; i++)
	{
		if (m_inputMap.keyMap[i] == (uint8)eInputState::Pressed)
			m_inputMap.keyMap[i] = (uint8)eInputState::Down;

		if (m_inputMap.keyMap[i] == (uint8)eInputState::Released)
			m_inputMap.keyMap[i] = (uint8)eInputState::None;
	}
	for (size_t i = 0; i < (uint32)eMouseMapping::Count; i++)
	{
		if (m_inputMap.mouse.keys[i] == (uint8)eInputState::Pressed)
			m_inputMap.mouse.keys[i] = (uint8)eInputState::Down;

		if (m_inputMap.mouse.keys[i] == (uint8)eInputState::Released)
			m_inputMap.mouse.keys[i] = (uint8)eInputState::None;
	}
}

void Hail::InputHandler::UpdateGamepads()
{
	for (size_t i = 0; i < 4; i++)
	{
		m_pGamepads[i]->Update();
	}
}

void InputHandler::ResetKeyStates()
{
	for (size_t i = 0; i < 0xff; i++)
		m_inputMap.keyMap[i] = (uint8)eInputState::None;

	for (size_t i = 0; i < (uint32)eMouseMapping::Count; i++)
		m_inputMap.mouse.keys[i] = (uint8)eInputState::None;

	for (size_t i = 0; i < 4; i++)
		m_pGamepads[i]->ResetStates();
}

Gamepad* Hail::InputHandler::GetGamePad(int gamepadIndex) const
{
	if (m_pGamepads[gamepadIndex]->GetisActive())
		return m_pGamepads[gamepadIndex];
	return nullptr;
}

bool Hail::InputHandler::IsGamepadActive(int gamepadIndex) const
{
	//TODO: Add assert on index
	if (Gamepad* pGamepad = m_pGamepads[gamepadIndex])
	{
		return pGamepad->GetisActive();
	}
	return false;
}

void Hail::InputHandler::SetGamepadTriggerButtonThresholds(float thresholdRange)
{
	for (size_t i = 0; i < 4; i++)
	{
		m_pGamepads[i]->SetTriggerButtonThreshold(thresholdRange);
	}
}

Hail::Gamepad::Gamepad(int index) :
	m_currentGamepadDirections({ 0.0, 0.0 }),
	m_currentButtonStates(eInputState::None),
	m_triggerStates(0.0),
	m_bIsActive(false),
	m_gamepadIndex(index),
	m_triggerButtonThresholdRange(0.3f),
	m_joystickDeadzone(0.2f)
{
}

int Hail::Gamepad::GetIndex()
{
	return m_gamepadIndex;
}

eInputState Hail::Gamepad::GetButtonInputState(eGamepadInputMapping inputKey) const
{
	return m_currentButtonStates[(uint8)inputKey];
}

glm::vec2 Hail::Gamepad::GetJoyStickDirection(uint32 leftOrRightStick) const
{
	return m_currentGamepadDirections[leftOrRightStick];
}

float Hail::Gamepad::GetTriggerInputState(uint32 leftOrRightTrigger) const
{
	return m_triggerStates[leftOrRightTrigger];
}

void Hail::Gamepad::ResetStates()
{
	for (size_t i = 0; i < 2; i++)
	{
		m_currentGamepadDirections[i] = Vec2Zero;
		m_triggerStates[i] = 0.0f;
	}

	for (size_t i = 0; i < (uint8)eGamepadInputMapping::Count; i++)
	{
		m_currentButtonStates[i] = eInputState::None;
	}

}
