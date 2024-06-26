#pragma once
#include "../ApplicationWindow.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files

namespace Hail
{
	class Windows_InputHandler;

	class Windows_ApplicationWindow : public ApplicationWindow
	{
	public:
		//recieve windows messages from the operating system
		static LRESULT CALLBACK WinProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
		bool Init(StartupAttributes startupData, Hail::InputHandler* inputHandler) final;

		HWND GetWindowHandle() { return m_windowHandle; }
		HINSTANCE GetAppModuleHandle() { return m_windowModule; }

		void SetApplicationSettings(Hail::ApplicationMessage message) final;

		void ApplicationUpdateLoop() final;

		glm::uvec2 GetWindowResolution() final;
		glm::uvec2 GetWindowPosition() final;
		glm::uvec2 GetMonitorResolution() final;

	private:

		void InternalSetWindowPos();

		Hail::InputHandler* m_inputHandler = nullptr;
		Hail::Windows_InputHandler* m_windowsInputHandler = nullptr;

		HWND m_windowHandle;
		HINSTANCE m_windowModule;
		glm::uvec2 m_defaultWindowPosition;
		glm::uvec2 m_borderSize;
	};
}


