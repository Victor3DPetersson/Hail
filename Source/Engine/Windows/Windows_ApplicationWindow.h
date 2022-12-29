#pragma once
#include "../ApplicationWindow.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files

class Windows_InputHandler;

class Windows_ApplicationWindow : public ApplicationWindow
{
public:
	//recieve windows messages from the operating system
	static LRESULT CALLBACK WinProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
	bool Init(StartupAttributes startupData, InputHandler* inputHandler) override;

	HWND GetWindowHandle() { return m_windowHandle; }

	void SetApplicationSettings(ApplicationMessage message) override;

	void ApplicationUpdateLoop() override;

	Vector2ui GetWindowResolution() override;
	Vector2ui GetWindowPosition() override;
	Vector2ui GetMonitorResolution() override;

private:
	
	void InternalSetWindowPos();

	InputHandler* m_inputHandler = nullptr;
	Windows_InputHandler* m_windowsInputHandler = nullptr;

	HWND m_windowHandle;
	Vector2ui m_defaultWindowPosition;
	Vector2ui m_borderSize;
};
