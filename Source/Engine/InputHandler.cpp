#include "Engine_PCH.h"
#include "InputHandler.h"

void InputHandler::Update()
{
    m_scrollFactor = 0;
    m_mouseDelta = Vector2ui();
    memset(&m_inputMaps.keyDownMap, 0, sizeof(char) * 0xff);
}

bool InputHandler::IsKeyHold(const int keyCode) const
{
    return m_inputMaps.keyHoldMap[keyCode];
}

bool InputHandler::IsKeyDown(const int keyCode) const
{
    return m_inputMaps.keyDownMap[keyCode];
}

bool InputHandler::IsKeyUp(const int keyCode) const
{
    return m_inputMaps.keyUpMap[keyCode];
}
