#pragma once

#include "Input/InputHandler.h"
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")
class Windows_ApplicationWindow;
namespace Hail
{

	class Windows_Gamepad final : public Gamepad
	{
	public:
		explicit Windows_Gamepad(int index);
		void Update() final;

		bool GetControllerState() final;
		bool Connected() final;

	private:

		XINPUT_STATE m_windowsGamePadState;

	};

	class Windows_InputHandler : public InputHandler
	{
	protected:
		void SetWindowHandle(HWND handle);
	public:
		Windows_InputHandler() = default;
		void InitInputMapping() final;
		friend class Windows_ApplicationWindow;

		void ShowCursor(bool visibilityState) const override;
		void SetMousePos(glm::uvec2 windowPosition) override;
		void LockMouseToWindow(bool lockMouse) override;
		void ReadInputEvents(UINT message, WPARAM wParam, LPARAM lParam);
	private:

		HCURSOR m_applicationCursor{};
		HWND m_windowHandle{};
	};
}
