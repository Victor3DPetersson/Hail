#pragma once
#include <stdint.h>
#include "Types.h"

namespace Hail
{
	constexpr uint32 MAX_NUMBER_OF_SPRITES = 1024u;
	constexpr uint32 MAX_NUMBER_OF_TEXT_COMMANDS = 128u;
	constexpr uint32 MAX_NUMBER_OF_DEBUG_LINES = 16392u;
	constexpr uint32 MAX_NUMBER_OF_DEBUG_CIRCLES = 16392u;
	constexpr uint32 MAX_NUMBER_OF_2D_RENDER_COMMANDS = MAX_NUMBER_OF_SPRITES + MAX_NUMBER_OF_TEXT_COMMANDS;

}

