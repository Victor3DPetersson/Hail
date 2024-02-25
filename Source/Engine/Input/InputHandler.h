#pragma once

#include "InputMappings.h"
#include "Containers\StaticArray\StaticArray.h"
class ApplicationWindow;

namespace Hail
{
	class Gamepad
	{
	public:
		// TODO call constructor
		explicit Gamepad(int index);

		virtual bool GetControllerState() = 0; 
		virtual void Update() = 0;

		// Index from 0 - 3
		int GetIndex();
		// The controller will be active if it has recieved any input since the last frame
		bool GetisActive() const { return m_bIsActive; }
		virtual bool Connected() = 0;        // Return true if gamepad is connected

		eInputState GetButtonInputState(eGamepadInputMapping inputKey) const;
		// L == 0 | R == 1
		glm::vec2 GetJoyStickDirection(uint32 leftOrRightStick) const;
		// L == 0 | R == 1
		float GetTriggerInputState(uint32 leftOrRightTrigger) const;

		void SetTriggerButtonThreshold(float thresholdRange) { m_triggerButtonThresholdRange = thresholdRange; }
		void SetJoystickThresholdRange(float thresholdRange) { m_joystickDeadzone = thresholdRange; }

		void ResetStates();

	protected:

		StaticArray<glm::vec2, 2> m_currentGamepadDirections; // L 0 | R 1
		StaticArray<eInputState, (uint8)eGamepadInputMapping::Count> m_currentButtonStates;
		StaticArray<float, 2> m_triggerStates; // L trigger 0 | R trigger 1

		int m_gamepadIndex;   // Gamepad index (eg. 0,1,2,3)
		bool m_bIsActive;
		float m_triggerButtonThresholdRange;
		float m_joystickDeadzone;
	};


	class InputHandler
	{
	public:
		friend class ApplicationWindow;
		InputHandler();
		virtual void InitInputMapping() = 0;
		void UpdateKeyStates();
		void UpdateGamepads();
		//Will be called when leaving focus of the application so that keys get reset when tabbing out
		void ResetKeyStates();

		virtual void ShowCursor(bool visibilityState) const = 0;
		virtual void SetMousePos(glm::uvec2 windowPosition) = 0;
		virtual void LockMouseToWindow(bool lockMouse) = 0;

		InputMapping& GetInputMapping() { return m_inputMapping; }
		InputMap& GetInputMap() { return m_inputMap; }

		Gamepad* GetGamePad(int gamepadIndex) const;

		bool IsGamepadActive(int gamepadIndex) const;

		void LockMousePos() { m_cursorLock = true; }
		void UnlockMousePos() { m_cursorLock = false; }
		const bool GetCursorLock() const { return m_cursorLock; }

		void SetGamepadTriggerButtonThresholds(float thresholdRange);
		void SetGamepadJoystickThresholdRange(float thresholdRange);

	protected:

		Gamepad* m_pGamepads[4];

		InputMap m_inputMap;
		InputMapping m_inputMapping;
		bool m_cursorLock;
		bool m_inputIsPaused;
		bool m_mainInputIsGamepad;
	};
}

