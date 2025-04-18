#pragma once
#include "glm\vec2.hpp"
#include "Types.h"
namespace Hail
{
	enum class eInputState
	{
		None,
		Pressed,
		Down,
		Released,
	};
	enum class eMouseMapping
	{
		LMB,
		RMB,
		MMB,
		Count
	};

	struct MouseMap
	{
		uint8 keys[(uint32)eMouseMapping::Count];
		glm::uvec2 mousePos;
		glm::vec2 normalizedPos;
		glm::vec2 mouseDelta;
		float scrollDelta = 0;
	};

	struct InputMap
	{
		unsigned char keyMap[0xff];
		MouseMap mouse;
	};

	enum class eGamepadInputMapping : uint8
	{
		A,
		B,
		X,
		Y,
		DPadUp,
		DPadDown,
		DPadLeft,
		DPadRight,
		L_Bumper,
		R_Bumper,
		L_Thumstick,
		R_Thumbstick,
		Start,
		Back,
		L_Trigger,
		R_Trigger,
		Count
	};

	//struct GamepadInputMapping
	//{
	//	uint16 A{};
	//	uint16 B{};
	//	uint16 X{};
	//	uint16 Y{};

	//	uint16 DPad_Up{};
	//	uint16 DPad_Down{};
	//	uint16 DPad_Left{};
	//	uint16 DPad_Right{};

	//	uint16 L_Bumper{};
	//	uint16	R_Bumber{};
	//	uint16 L_Thumbstick{};
	//	uint16 R_Thumbstick;

	//	uint16 Start{};
	//	uint16 Back{};
	//};

	struct InputMapping
	{
		unsigned short CTRL = 0;
		unsigned short SHFT = 0;
		unsigned short TAB = 0;
		unsigned short ESC = 0;
		unsigned short ALT = 0;
		unsigned short SPACE = 0;
		unsigned short ENTER = 0;
		unsigned short BKSPC = 0;
		unsigned short DEL = 0;
		unsigned short INS = 0;
		unsigned short END = 0;
		unsigned short HME = 0;
		unsigned short PGUP = 0;//page up
		unsigned short PGDN = 0;//page down
		unsigned short PRTSC = 0;//print screen

		unsigned short F1 = 0;
		unsigned short F2 = 0;
		unsigned short F3 = 0;
		unsigned short F4 = 0;
		unsigned short F5 = 0;
		unsigned short F6 = 0;
		unsigned short F7 = 0;
		unsigned short F8 = 0;
		unsigned short F9 = 0;
		unsigned short F10 = 0;
		unsigned short F11 = 0;
		unsigned short F12 = 0;
		unsigned short F13 = 0;
		unsigned short F14 = 0;
		unsigned short F15 = 0;
		unsigned short F16 = 0;
		unsigned short F17 = 0;
		unsigned short F18 = 0;
		unsigned short F19 = 0;
		unsigned short F20 = 0;
		unsigned short F21 = 0;
		unsigned short F22 = 0;
		unsigned short F23 = 0;
		unsigned short F24 = 0;

		unsigned short NMB_1 = 0;
		unsigned short NMB_2 = 0;
		unsigned short NMB_3 = 0;
		unsigned short NMB_4 = 0;
		unsigned short NMB_5 = 0;
		unsigned short NMB_6 = 0;
		unsigned short NMB_7 = 0;
		unsigned short NMB_8 = 0;
		unsigned short NMB_9 = 0;
		unsigned short NMB_0 = 0;

		unsigned short Q = 0;
		unsigned short W = 0;
		unsigned short E = 0;
		unsigned short R = 0;
		unsigned short T = 0;
		unsigned short Y = 0;
		unsigned short U = 0;
		unsigned short I = 0;
		unsigned short O = 0;
		unsigned short P = 0;
		unsigned short A = 0;
		unsigned short S = 0;
		unsigned short D = 0;
		unsigned short F = 0;
		unsigned short G = 0;
		unsigned short H = 0;
		unsigned short J = 0;
		unsigned short K = 0;
		unsigned short L = 0;
		unsigned short Z = 0;
		unsigned short X = 0;
		unsigned short C = 0;
		unsigned short V = 0;
		unsigned short B = 0;
		unsigned short N = 0;
		unsigned short M = 0;

		unsigned short COMMA = 0; //-
		unsigned short ADD = 0;
		unsigned short SUBTRACT = 0;
		unsigned short DIVIDE = 0;
		unsigned short DOT = 0;
	};
}
