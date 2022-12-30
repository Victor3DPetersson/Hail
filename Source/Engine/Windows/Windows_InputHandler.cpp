#include "Engine_PCH.h"
#include "Windows_InputHandler.h"

void Windows_InputHandler::SetWindowHandle(HWND handle)
{
	m_windowHandle = handle;
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
       m_inputMaps.keyDownMap[wParam] = 1;
       m_inputMaps.keyHoldMap[wParam] = 1;
        return;
    case WM_KEYUP:
        m_inputMaps.keyHoldMap[wParam] = 0;
        m_inputMaps.keyUpMap[wParam] = 1;
        return;
    case WM_MOUSEMOVE:
    {
        POINT cursorPosition{ LOWORD(lParam) , HIWORD(lParam) };
        m_mousePosition.x = cursorPosition.x;
        m_mousePosition.y = cursorPosition.y;
        return;
    }
    case WM_LBUTTONDOWN:
        m_inputMaps.keyDownMap[VK_LBUTTON] = 1;
        m_inputMaps.keyHoldMap[VK_LBUTTON] = 1;
        return;
    case WM_LBUTTONUP:
        m_inputMaps.keyHoldMap[VK_LBUTTON] = 0;
        m_inputMaps.keyUpMap[VK_LBUTTON] = 1;
        return;
    case WM_RBUTTONDOWN:
        m_inputMaps.keyDownMap[VK_RBUTTON] = 1;
        m_inputMaps.keyHoldMap[VK_RBUTTON] = 1;
        return;
    case WM_RBUTTONUP:
        m_inputMaps.keyHoldMap[VK_RBUTTON] = 0;
        m_inputMaps.keyUpMap[VK_RBUTTON] = 1;
        return;
    case WM_MBUTTONDOWN:
        m_inputMaps.keyDownMap[VK_MBUTTON] = 1;
        m_inputMaps.keyHoldMap[VK_MBUTTON] = 1;
        return;
    case WM_MBUTTONUP:
        m_inputMaps.keyHoldMap[VK_MBUTTON] = 0;
        m_inputMaps.keyUpMap[VK_MBUTTON] = 1;
        return;
    case WM_MOUSEWHEEL:
        m_scrollFactor = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
        return;

    case WM_INPUT:
        unsigned size = sizeof(RAWINPUT);
        static RAWINPUT raw[sizeof(RAWINPUT)];
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

        if (raw->header.dwType == RIM_TYPEMOUSE) {
            m_mouseDelta.x = raw->data.mouse.lLastX;
            m_mouseDelta.y = raw->data.mouse.lLastY;
        }
        return;
    }
}