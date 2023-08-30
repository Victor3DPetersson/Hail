#pragma once
#include "glm\vec2.hpp"
#include <type_traits>

using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

using int8 = char;
using int16 = short;
using int32 = int;
using int64 = long long;

using float32 = float;
using float64 = double;

template <typename E>
constexpr typename std::underlying_type<E>::type ToUnderlyingType(E e) noexcept 
{
	return static_cast<typename std::underlying_type<E>::type>(e);
}

template <typename Enum>
constexpr typename Enum ToEnum(uint32_t index)
{
	//TODO: Add assert here is index is out of range
	return static_cast<Enum>(index);
}

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


inline glm::uvec2 ResolutionFromEnum(RESOLUTIONS res)
{
	glm::uvec2 resolution;
	switch (res)
	{
	case RESOLUTIONS::RES2160:
		resolution = glm::uvec2(3840, 2160);
		break;
	case RESOLUTIONS::RES1440:
		resolution = glm::uvec2(2560, 1440);
		break;
	case RESOLUTIONS::RES1080:
		resolution = glm::uvec2(1920, 1080);
		break;
	case RESOLUTIONS::RES720:
		resolution = glm::uvec2(1280, 720);
		break;
	case RESOLUTIONS::RES480:
		resolution = glm::uvec2(854, 480);
		break;
	case RESOLUTIONS::RES360:
		resolution = glm::uvec2(640, 360);
		break;
	default:
		break;
	}
	return resolution;
}

#define SAFEDELETE(ptr) if(ptr) { delete ptr; ptr = nullptr; }
#define SAFEDELETE_ARRAY(ptr) if(ptr) { delete[] ptr; ptr = nullptr; }

