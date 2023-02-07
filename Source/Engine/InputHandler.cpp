#include "Engine_PCH.h"
#include "InputHandler.h"

void InputHandler::ResetKeyStates()
{
	for (size_t i = 0; i < 0xff; i++)
	{
		if (m_inputMap.keyMap[i] == Hail::KEY_RELEASED)
		{
			m_inputMap.keyMap[i] = Hail::KEY_NONE;
		}
	}
	for (size_t i = 0; i < Hail::MOUSE_KEY_COUNT; i++)
	{
		if (m_inputMap.mouse.keys[i] == Hail::KEY_RELEASED)
		{
			m_inputMap.mouse.keys[i] = Hail::KEY_NONE;
		}
	}
	//memset(m_inputMap.keyMap, Hail::KEY_NONE, sizeof(char) * 0xff);
	//memset(m_inputMap.mouse.keys, Hail::KEY_NONE, sizeof(char) * Hail::MOUSE_KEY_COUNT);
}
