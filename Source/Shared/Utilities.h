#pragma once

#include "Vector2.hpp"

enum class RESOLUTIONS
{
	RES2160,
	RES1440,
	RES1080,
	RES720,
	RES480,
	RES360,
	COUNT
};


inline Vector2ui ResolutionFromEnum(RESOLUTIONS res)
{
	Vector2ui resolution;
	switch (res)
	{
	case RESOLUTIONS::RES2160:
		resolution = Vector2ui(3840, 2160);
		break;
	case RESOLUTIONS::RES1440:
		resolution = Vector2ui(2560, 1440);
		break;
	case RESOLUTIONS::RES1080:
		resolution = Vector2ui(1920, 1080);
		break;
	case RESOLUTIONS::RES720:
		resolution = Vector2ui(1280, 720);
		break;
	case RESOLUTIONS::RES480:
		resolution = Vector2ui(854, 480);
		break;
	case RESOLUTIONS::RES360:
		resolution = Vector2ui(640, 360);
		break;
	default:
		break;
	}
	return resolution;
}
