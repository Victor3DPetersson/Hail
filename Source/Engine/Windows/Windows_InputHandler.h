#pragma once

#include "InputHandler.h"
#include <Windows.h>

class Windows_ApplicationWindow;

class Windows_InputHandler : public InputHandler
{
protected:
	void SetWindowHandle(HWND handle);
public:
	Windows_InputHandler() = default;

	friend class Windows_ApplicationWindow;

	void ShowCursor(bool visibilityState) const override;
	void SetMousePos(glm::uvec2 windowPosition) override;
	void LockMouseToWindow(bool lockMouse) override;
	void ReadInputEvents(UINT message, WPARAM wParam, LPARAM lParam);
private:
	HCURSOR m_applicationCursor;
	HWND m_windowHandle;
};