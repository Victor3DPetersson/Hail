#pragma once

#include "InputMappings.h"
class ApplicationWindow;


class InputHandler
{
public:
	friend class ApplicationWindow;
	InputHandler() = default;
	virtual void InitInputMapping() = 0;
	void ResetKeyStates();

	virtual void ShowCursor(bool visibilityState) const = 0;
	virtual void SetMousePos(glm::uvec2 windowPosition) = 0;
	virtual void LockMouseToWindow(bool lockMouse) = 0;


	Hail::InputMapping& GetInputMapping() { return m_inputMappings; }
	Hail::InputMap& GetInputMap() { return m_inputMap; }

	void LockMousePos() { m_cursorLock = true; }
	void UnlockMousePos() { m_cursorLock = false; }
	const bool GetCursorLock() const { return m_cursorLock; }

protected:



	Hail::InputMap m_inputMap;
	Hail::InputMapping m_inputMappings;
	bool m_cursorLock = false;
	bool m_inputIsPaused = false;
};