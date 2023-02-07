#include "Engine_PCH.h"
#include "Windows_InputHandler.h"

#include "DebugMacros.h"

void Windows_InputHandler::SetWindowHandle(HWND handle)
{
	m_windowHandle = handle;
}

void Windows_InputHandler::InitInputMapping()
{
    m_inputMappings.Q = 0x51;
    m_inputMappings.W = 0x57;
    m_inputMappings.E = 0x45;
    m_inputMappings.R = 0x52;
    m_inputMappings.T = 0x54;
    m_inputMappings.Y = 0x59;
    m_inputMappings.U = 0x55;
    m_inputMappings.I = 0x49;
    m_inputMappings.O = 0x4F;
    m_inputMappings.P = 0x50;
    m_inputMappings.A = 0x41;
    m_inputMappings.S = 0x53;
    m_inputMappings.D = 0x44;
    m_inputMappings.F = 0x46;
    m_inputMappings.G = 0x47;
    m_inputMappings.H = 0x48;
    m_inputMappings.J = 0x4A;
    m_inputMappings.K = 0x4B;
    m_inputMappings.L = 0x4C;
    m_inputMappings.Z = 0x5A;
    m_inputMappings.X = 0x58;
    m_inputMappings.C = 0x43;
    m_inputMappings.V = 0x56;
    m_inputMappings.B = 0x42;
    m_inputMappings.N = 0x4E;
    m_inputMappings.M = 0x4D;

    m_inputMappings.CTRL = VK_CONTROL;
    m_inputMappings.SHFT = VK_SHIFT;
    m_inputMappings.TAB = VK_TAB;
    m_inputMappings.ESC = VK_ESCAPE;
    m_inputMappings.ALT = VK_MENU;
    m_inputMappings.SPACE = VK_SPACE;
    m_inputMappings.ENTER = VK_RETURN;
    m_inputMappings.BKSPC = VK_BACK;
    m_inputMappings.DEL = VK_DELETE;
    m_inputMappings.INS = VK_INSERT;
    m_inputMappings.END = VK_END;
    m_inputMappings.HME = VK_HOME;
    m_inputMappings.PGUP = VK_PRIOR;//page up
    m_inputMappings.PGDN = VK_NEXT;//page down
    m_inputMappings.PRTSC = VK_PRINT;//print screen

    m_inputMappings.F1 = VK_F1;
    m_inputMappings.F2 = VK_F2;
    m_inputMappings.F3 = VK_F3;
    m_inputMappings.F4 = VK_F4;
    m_inputMappings.F5 = VK_F5;
    m_inputMappings.F6 = VK_F6;
    m_inputMappings.F7 = VK_F7;
    m_inputMappings.F8 = VK_F8;
    m_inputMappings.F9 = VK_F9;
    m_inputMappings.F10 = VK_F10;
    m_inputMappings.F11 = VK_F11;
    m_inputMappings.F12 = VK_F12;
    m_inputMappings.F13 = VK_F13;
    m_inputMappings.F14 = VK_F14;
    m_inputMappings.F15 = VK_F15;
    m_inputMappings.F16 = VK_F16;
    m_inputMappings.F17 = VK_F17;
    m_inputMappings.F18 = VK_F18;
    m_inputMappings.F19 = VK_F19;
    m_inputMappings.F20 = VK_F20;
    m_inputMappings.F21 = VK_F21;
    m_inputMappings.F22 = VK_F22;
    m_inputMappings.F23 = VK_F23;
    m_inputMappings.F24 = VK_F24;

    m_inputMappings.NMB_1 = 0x30;//VK_NUMPAD1;
    m_inputMappings.NMB_2 = 0x31;//VK_NUMPAD2;
    m_inputMappings.NMB_3 = 0x32;//VK_NUMPAD3;
    m_inputMappings.NMB_4 = 0x33;//VK_NUMPAD4;
    m_inputMappings.NMB_5 = 0x34;//VK_NUMPAD5;
    m_inputMappings.NMB_6 = 0x35;//VK_NUMPAD6;
    m_inputMappings.NMB_7 = 0x36;//VK_NUMPAD7;
    m_inputMappings.NMB_8 = 0x37;//VK_NUMPAD8;
    m_inputMappings.NMB_9 = 0x38;//VK_NUMPAD9;
    m_inputMappings.NMB_0 = 0x39;//VK_NUMPAD0;


    m_inputMappings.COMMA = VK_SEPARATOR; //-
    m_inputMappings.ADD = VK_ADD;
    m_inputMappings.SUBTRACT = VK_SUBTRACT;
    m_inputMappings.DIVIDE = VK_DIVIDE;
    m_inputMappings.DOT = VK_DECIMAL;
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
            if (m_inputMap.keyMap[wParam] == Hail::KEY_NONE)
            {
                m_inputMap.keyMap[wParam] = Hail::KEY_PRESSED;
            }
            return;
        case WM_KEYUP:
            m_inputMap.keyMap[wParam] = Hail::KEY_RELEASED;
            return;
        case WM_MOUSEMOVE:
        {
            POINT cursorPosition{ LOWORD(lParam) , HIWORD(lParam) };
            m_inputMap.mouse.mousePos.x = cursorPosition.x;
            m_inputMap.mouse.mousePos.y = cursorPosition.y;
            return;
        }
        case WM_LBUTTONDOWN:
            if (m_inputMap.mouse.keys[Hail::LMB] == Hail::KEY_NONE)
            {
                m_inputMap.mouse.keys[Hail::LMB] = Hail::KEY_PRESSED;
            }
            return;
        case WM_LBUTTONUP:
            m_inputMap.mouse.keys[Hail::LMB] = Hail::KEY_RELEASED;
            return;
        case WM_RBUTTONDOWN:
            if (m_inputMap.mouse.keys[Hail::RMB] == Hail::KEY_NONE)
            {
                m_inputMap.mouse.keys[Hail::RMB] = Hail::KEY_PRESSED;
            }
            return;
        case WM_RBUTTONUP:
            m_inputMap.mouse.keys[Hail::RMB] = Hail::KEY_RELEASED;
            return;
        case WM_MBUTTONDOWN:
            if (m_inputMap.mouse.keys[Hail::MMB] == Hail::KEY_NONE)
            {
                m_inputMap.mouse.keys[Hail::MMB] = Hail::KEY_PRESSED;
            }
            return;
        case WM_MBUTTONUP:
            m_inputMap.mouse.keys[Hail::MMB] = Hail::KEY_RELEASED;
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