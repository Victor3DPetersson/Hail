#pragma once

#include "glm\vec2.hpp"

class ApplicationWindow;

class InputHandler
{
public:
	friend class ApplicationWindow;
	InputHandler() = default;

	void Update();

	virtual void ShowCursor(bool visibilityState) const = 0;
	virtual void SetMousePos(glm::uvec2 windowPosition) = 0;
	virtual void LockMouseToWindow(bool lockMouse) = 0;

	bool IsKeyHold(const int keyCode) const;
	bool IsKeyUp(const int keyCode) const;
	bool IsKeyDown(const int keyCode) const;

	//Mouse Events

	glm::uvec2 GetMousePosition() const { return m_mousePosition; }
	glm::uvec2 GetMouseDelta() const { return m_mouseDelta; }

	float GetScroll() const { return m_scrollFactor; }//-1.0 - 1.0

	void LockMousePos() { m_cursorLock = true; }
	void UnlockMousePos() { m_cursorLock = false; }
	const bool GetCursorLock() const { return m_cursorLock; }

protected:
	struct InputMaps
	{
		char keyUpMap[0xff];
		char keyDownMap[0xff];
		char keyHoldMap[0xff];
	};

	glm::uvec2 m_mousePosition;
	glm::uvec2 m_mouseDelta;
	float m_scrollFactor;

	InputMaps m_inputMaps;

	bool m_cursorLock = false;
	bool m_inputIsPaused = false;
};