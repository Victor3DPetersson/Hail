#pragma once

namespace Hail
{
	enum class eInputAction : uint8
	{
		PlayerMoveUp,
		PlayerMoveRight,
		PlayerMoveLeft,
		PlayerMoveDown,
		PlayerMoveJoystick,
		PlayerAction1,
		PlayerAction2,
		PlayerPause, // This action must be the last action of the player actions and must always be present
		DebugAction1,
		DebugAction2,
		DebugAction3,
		DebugAction4,
		InputCount// Must be the last 
	};
}
