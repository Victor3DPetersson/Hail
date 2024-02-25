#include "Engine_PCH.h"
#include "Windows_InputHandler.h"

#include "DebugMacros.h"

#include <concrt.h>
#include <winrt/Windows.Gaming.Input.h>

#include "MathUtils.h"
#include "glm\geometric.hpp"

#include <cstdint>
#include <iostream>
#include <roapi.h>
#include <wrl.h>
#include "windows.gaming.input.h"

using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Gaming::Input;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

#pragma comment(lib, "runtimeobject.lib")

using namespace Hail;
using namespace winrt;
//using namespace Windows::Gaming::Input;



void Windows_InputHandler::SetWindowHandle(HWND handle)
{
	m_windowHandle = handle;
}

void Windows_InputHandler::InitInputMapping()
{
    //auto hr = RoInitialize(RO_INIT_MULTITHREADED);
    //assert(SUCCEEDED(hr));

    //ComPtr<IGamepadStatics> gamepadStatics;
    //hr = RoGetActivationFactory(HStringReference(L"Windows.Gaming.Input.Gamepad").Get(), __uuidof(IGamepadStatics), &gamepadStatics);
    //assert(SUCCEEDED(hr));

    //ComPtr<IVectorView<Gamepad*>> gamepads;
    //hr = gamepadStatics->get_Gamepads(&gamepads);
    //assert(SUCCEEDED(hr));

    //uint32_t gamepadCount;
    //hr = gamepads->get_Size(&gamepadCount);
    //assert(SUCCEEDED(hr));

    //for (uint32_t i = 0; i < gamepadCount; i++)
    //{
    //    ComPtr<IGamepad> gamepad;
    //    hr = gamepads->GetAt(i, &gamepad);
    //    assert(SUCCEEDED(hr));

    //    GamepadReading gamepadReading;
    //    hr = gamepad->GetCurrentReading(&gamepadReading);
    //    assert(SUCCEEDED(hr));

    //    std::cout << "Gamepad " << i + 1 << " buttons value is: " << gamepadReading.Buttons << std::endl;
    //}

    
    //std::vector<Gamepad> myGamepads;
    //concurrency::critical_section myLock{};
    //auto gamepads = Gamepad::Gamepads();


    //for (int i = 0; i < gamepads.Size(); i++)
    //{
    //    auto const& gamepad = gamepads.GetAt(i);
    //    // Test whether the gamepad is already in myGamepads; if it isn't, add it.
    //    concurrency::critical_section::scoped_lock lock{ myLock };
    //    auto it{ std::find(begin(myGamepads), end(myGamepads), gamepad) };

    //    if (it == end(myGamepads))
    //    {
    //        // This code assumes that you're interested in all gamepads.
    //        myGamepads.push_back(gamepad);
    //    }
    //}

    m_inputMapping.Q = 0x51;
    m_inputMapping.W = 0x57;
    m_inputMapping.E = 0x45;
    m_inputMapping.R = 0x52;
    m_inputMapping.T = 0x54;
    m_inputMapping.Y = 0x59;
    m_inputMapping.U = 0x55;
    m_inputMapping.I = 0x49;
    m_inputMapping.O = 0x4F;
    m_inputMapping.P = 0x50;
    m_inputMapping.A = 0x41;
    m_inputMapping.S = 0x53;
    m_inputMapping.D = 0x44;
    m_inputMapping.F = 0x46;
    m_inputMapping.G = 0x47;
    m_inputMapping.H = 0x48;
    m_inputMapping.J = 0x4A;
    m_inputMapping.K = 0x4B;
    m_inputMapping.L = 0x4C;
    m_inputMapping.Z = 0x5A;
    m_inputMapping.X = 0x58;
    m_inputMapping.C = 0x43;
    m_inputMapping.V = 0x56;
    m_inputMapping.B = 0x42;
    m_inputMapping.N = 0x4E;
    m_inputMapping.M = 0x4D;

    m_inputMapping.CTRL = VK_CONTROL;
    m_inputMapping.SHFT = VK_SHIFT;
    m_inputMapping.TAB = VK_TAB;
    m_inputMapping.ESC = VK_ESCAPE;
    m_inputMapping.ALT = VK_MENU;
    m_inputMapping.SPACE = VK_SPACE;
    m_inputMapping.ENTER = VK_RETURN;
    m_inputMapping.BKSPC = VK_BACK;
    m_inputMapping.DEL = VK_DELETE;
    m_inputMapping.INS = VK_INSERT;
    m_inputMapping.END = VK_END;
    m_inputMapping.HME = VK_HOME;
    m_inputMapping.PGUP = VK_PRIOR;//page up
    m_inputMapping.PGDN = VK_NEXT;//page down
    m_inputMapping.PRTSC = VK_PRINT;//print screen

    m_inputMapping.F1 = VK_F1;
    m_inputMapping.F2 = VK_F2;
    m_inputMapping.F3 = VK_F3;
    m_inputMapping.F4 = VK_F4;
    m_inputMapping.F5 = VK_F5;
    m_inputMapping.F6 = VK_F6;
    m_inputMapping.F7 = VK_F7;
    m_inputMapping.F8 = VK_F8;
    m_inputMapping.F9 = VK_F9;
    m_inputMapping.F10 = VK_F10;
    m_inputMapping.F11 = VK_F11;
    m_inputMapping.F12 = VK_F12;
    m_inputMapping.F13 = VK_F13;
    m_inputMapping.F14 = VK_F14;
    m_inputMapping.F15 = VK_F15;
    m_inputMapping.F16 = VK_F16;
    m_inputMapping.F17 = VK_F17;
    m_inputMapping.F18 = VK_F18;
    m_inputMapping.F19 = VK_F19;
    m_inputMapping.F20 = VK_F20;
    m_inputMapping.F21 = VK_F21;
    m_inputMapping.F22 = VK_F22;
    m_inputMapping.F23 = VK_F23;
    m_inputMapping.F24 = VK_F24;

    m_inputMapping.NMB_1 = VK_NUMPAD1;//0x30;
    m_inputMapping.NMB_2 = VK_NUMPAD2;//0x31;
    m_inputMapping.NMB_3 = VK_NUMPAD3;//0x32;
    m_inputMapping.NMB_4 = VK_NUMPAD4;//0x33;
    m_inputMapping.NMB_5 = VK_NUMPAD5;//0x34;
    m_inputMapping.NMB_6 = VK_NUMPAD6;//0x35;
    m_inputMapping.NMB_7 = VK_NUMPAD7;//0x36;
    m_inputMapping.NMB_8 = VK_NUMPAD8;//0x37;
    m_inputMapping.NMB_9 = VK_NUMPAD9;//0x38;
    m_inputMapping.NMB_0 = VK_NUMPAD0;//0x39;


    m_inputMapping.COMMA = VK_SEPARATOR; //-
    m_inputMapping.ADD = VK_ADD;
    m_inputMapping.SUBTRACT = VK_SUBTRACT;
    m_inputMapping.DIVIDE = VK_DIVIDE;
    m_inputMapping.DOT = VK_DECIMAL;

    for (int i = 0; i < 4; i++)
        m_pGamepads[i] = new Windows_Gamepad(i);

}

void Windows_InputHandler::ShowCursor(bool visibilityState) const
{
    if (visibilityState)
    {
        SetCursor(m_applicationCursor);
    }
    else
    {
        SetCursor(NULL);
    }
}

void Windows_InputHandler::SetMousePos(glm::uvec2 windowPosition)
{
    if (m_cursorLock) return;
    m_inputMap.mouse.mousePos = windowPosition;
    POINT tempPoint;
    tempPoint.x = (LONG)windowPosition.x;
    tempPoint.y = (LONG)windowPosition.y;
    ClientToScreen(m_windowHandle, &tempPoint);
    SetCursorPos(tempPoint.x, tempPoint.y);
}

void Windows_InputHandler::LockMouseToWindow(bool lockMouse)
{
    if (lockMouse)
    {
        RECT rect;
        GetClientRect(m_windowHandle, &rect);

        POINT upperL;
        upperL.x = rect.left;
        upperL.y = rect.top;

        POINT lowerR;
        lowerR.x = rect.right;
        lowerR.y = rect.bottom;
        MapWindowPoints(m_windowHandle, nullptr, &upperL, 1);
        MapWindowPoints(m_windowHandle, nullptr, &lowerR, 1);

        rect.left = upperL.x;
        rect.top = upperL.y;
        rect.right = lowerR.x;
        rect.bottom = lowerR.y;

        ClipCursor(&rect);
    }
    else
    {
        ClipCursor(nullptr);
    }
}


void Windows_InputHandler::ReadInputEvents(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_KEYDOWN:
            if (m_inputMap.keyMap[wParam] == (uint8)eInputState::None)
            {
                m_inputMap.keyMap[wParam] = (uint8)eInputState::Pressed;
            }
            return;
        case WM_KEYUP:
            m_inputMap.keyMap[wParam] = (uint8)eInputState::Released;
            return;
        case WM_MOUSEMOVE:
        {
            POINT cursorPosition{ LOWORD(lParam) , HIWORD(lParam) };
            m_inputMap.mouse.mousePos.x = cursorPosition.x;
            m_inputMap.mouse.mousePos.y = cursorPosition.y;
            return;
        }
        case WM_LBUTTONDOWN:
            if (m_inputMap.mouse.keys[(uint8)eMouseMapping::LMB] == (uint8)eInputState::None)
            {
                m_inputMap.mouse.keys[(uint8)eMouseMapping::LMB] = (uint8)eInputState::Pressed;
            }
            return;
        case WM_LBUTTONUP:
            m_inputMap.mouse.keys[(uint8)eMouseMapping::LMB] = (uint8)eInputState::Released;
            return;
        case WM_RBUTTONDOWN:
            if (m_inputMap.mouse.keys[(uint8)eMouseMapping::RMB] == (uint8)eInputState::None)
            {
                m_inputMap.mouse.keys[(uint8)eMouseMapping::RMB] = (uint8)eInputState::Pressed;
            }
            return;
        case WM_RBUTTONUP:
            m_inputMap.mouse.keys[(uint8)eMouseMapping::RMB] = (uint8)eInputState::Released;
            return;
        case WM_MBUTTONDOWN:
            if (m_inputMap.mouse.keys[(uint8)eMouseMapping::MMB] == (uint8)eInputState::None)
            {
                m_inputMap.mouse.keys[(uint8)eMouseMapping::MMB] = (uint8)eInputState::Pressed;
            }
            return;
        case WM_MBUTTONUP:
            m_inputMap.mouse.keys[(uint8)eMouseMapping::MMB] = (uint8)eInputState::Released;
            return;
        case WM_MOUSEWHEEL:
            m_inputMap.mouse.scrollDelta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam) / 120);
            return;
        case WM_INPUT:
        {
            unsigned size = sizeof(RAWINPUT);
            static RAWINPUT raw[sizeof(RAWINPUT)];
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));
            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                m_inputMap.mouse.mouseDelta.x = raw->data.mouse.lLastX;
                m_inputMap.mouse.mouseDelta.y = raw->data.mouse.lLastY;
            }
            return;
        }
    }
}

static const WORD XINPUT_Buttons[] = {
      XINPUT_GAMEPAD_A,
      XINPUT_GAMEPAD_B,
      XINPUT_GAMEPAD_X,
      XINPUT_GAMEPAD_Y,
      XINPUT_GAMEPAD_DPAD_UP,
      XINPUT_GAMEPAD_DPAD_DOWN,
      XINPUT_GAMEPAD_DPAD_LEFT,
      XINPUT_GAMEPAD_DPAD_RIGHT,
      XINPUT_GAMEPAD_LEFT_SHOULDER,
      XINPUT_GAMEPAD_RIGHT_SHOULDER,
      XINPUT_GAMEPAD_LEFT_THUMB,
      XINPUT_GAMEPAD_RIGHT_THUMB,
      XINPUT_GAMEPAD_START,
      XINPUT_GAMEPAD_BACK
};

Hail::Windows_Gamepad::Windows_Gamepad(int index) : Gamepad(index)
{
    ZeroMemory(&m_windowsGamePadState, sizeof(XINPUT_STATE));
    for (size_t i = 0; i < 2; i++)
    {
        m_currentGamepadDirections[i] = glm::vec2(0.0, 0.0);
        m_triggerStates[i] = 0.0f;
    }
    for (uint8 i = 0; i < (uint8)eGamepadInputMapping::Count; i++)
    {
        m_currentButtonStates = eInputState::None;
    }
}

void Hail::Windows_Gamepad::Update()
{
    if (!GetControllerState())
    {
        m_bIsActive = false;
        return;
    }

    bool isActiveThisFrame = false;

    uint16 buttonStates = m_windowsGamePadState.Gamepad.wButtons;
    for (uint8 i = 0; i < (uint8)eGamepadInputMapping::Count; i++)
    {
        bool isButtonPressed = false;
        if (i <= (uint8)eGamepadInputMapping::Back)
        {
            isButtonPressed = buttonStates & XINPUT_Buttons[i];
        }
        else
        {
            const float triggerState = GetTriggerInputState(i - (uint8)eGamepadInputMapping::L_Trigger);
            if (triggerState >= m_triggerButtonThresholdRange)
                isButtonPressed = true;
        }
        eInputState& currentState = m_currentButtonStates[i];
        const eInputState previousState = currentState;
        if (isButtonPressed)
        {
            if (currentState == eInputState::None)
                currentState = eInputState::Pressed;
            else if (currentState == eInputState::Pressed)
                currentState = eInputState::Down;

            if (currentState != eInputState::None)
                isActiveThisFrame = true;
        }
        else
        {
            if (currentState == eInputState::Pressed)
                currentState = eInputState::Released;
            else if (currentState == eInputState::Released)
                currentState = eInputState::None;
            else if (currentState == eInputState::Down)
                currentState = eInputState::Released;
        }

        isActiveThisFrame |= previousState != currentState;

    }

    {
        const float previousLTriggerState = m_triggerStates[0];
        const BYTE LTrigger = m_windowsGamePadState.Gamepad.bLeftTrigger;
        if (LTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            m_triggerStates[0] = LTrigger / 255.0f;
        else
            m_triggerStates[0] = 0.0f;
        isActiveThisFrame |= previousLTriggerState != m_triggerStates[0];

        const float previousRTriggerState = m_triggerStates[1];
        const BYTE RTrigger = m_windowsGamePadState.Gamepad.bRightTrigger;
        if (RTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            m_triggerStates[1] = RTrigger / 255.0f;
        else
            m_triggerStates[1] = 0.0f;
        isActiveThisFrame |= previousRTriggerState != m_triggerStates[1];
    }

    {
        // Obtain the X & Y axes of the stick
        const short lX = m_windowsGamePadState.Gamepad.sThumbLX;
        const short lY = m_windowsGamePadState.Gamepad.sThumbLY;
        m_currentGamepadDirections[0] = { (float)(lX) / 32768.0f, (float)(lY) / 32768.0f };
        if (glm::length(m_currentGamepadDirections[0]) < m_joystickDeadzone) m_currentGamepadDirections[0] = Vec2Zero;
        isActiveThisFrame |= Vec2Zero != m_currentGamepadDirections[0];


        const short rX = m_windowsGamePadState.Gamepad.sThumbRX;
        const short rY = m_windowsGamePadState.Gamepad.sThumbRY;
        m_currentGamepadDirections[1] = { (float)(rX) / 32768.0f, (float)(rY) / 32768.0f };
        if (glm::length(m_currentGamepadDirections[1]) < m_joystickDeadzone) m_currentGamepadDirections[1] = Vec2Zero;
        isActiveThisFrame |= Vec2Zero != m_currentGamepadDirections[1];


    }

    m_bIsActive = isActiveThisFrame;

}

bool Hail::Windows_Gamepad::GetControllerState()
{
    // Temporary XINPUT_STATE to return
    XINPUT_STATE gamepadState;

    // Zero memory
    ZeroMemory(&gamepadState, sizeof(XINPUT_STATE));

    // Get the state of the gamepad
    if (XInputGetState(m_gamepadIndex, &gamepadState) != ERROR_DEVICE_NOT_CONNECTED)
    {
        m_windowsGamePadState = gamepadState;
        return true;
    }
    return false;
}

bool Hail::Windows_Gamepad::Connected()
{
    XINPUT_STATE state;

    ZeroMemory(&state, sizeof(XINPUT_STATE));

    // Get the state of the gamepad
    DWORD Result = XInputGetState(m_gamepadIndex, &state);

    if (Result == ERROR_SUCCESS)
        return true;  // The gamepad is connected
    else
        return false; // The gamepad is not connected
}

