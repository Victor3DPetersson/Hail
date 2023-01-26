#pragma once
#include "Vertices.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"

//TODO: move this later
enum class BLEND_MODE : uint32_t
{
	NORMAL,
	ALPHABLEND,
	CUTOUT,
	ADDITIVE
};

struct Material
{
	String64 textures[4];
	BLEND_MODE blendMode;
};
