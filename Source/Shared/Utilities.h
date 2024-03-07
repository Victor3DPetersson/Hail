#pragma once
#include "glm\vec2.hpp"
#include "Types.h"
#include <type_traits>

namespace Hail
{
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

	inline glm::uvec2 ResolutionFromEnum(eResolutions res)
	{
		glm::uvec2 resolution;
		switch (res)
		{
		case eResolutions::res2160:
			resolution = glm::uvec2(3840, 2160);
			break;
		case eResolutions::res1440:
			resolution = glm::uvec2(2560, 1440);
			break;
		case eResolutions::res1080:
			resolution = glm::uvec2(1920, 1080);
			break;
		case eResolutions::res720:
			resolution = glm::uvec2(1280, 720);
			break;
		case eResolutions::res480:
			resolution = glm::uvec2(854, 480);
			break;
		case eResolutions::res360:
			resolution = glm::uvec2(640, 360);
			break;
		default:
			break;
		}
		return resolution;
	}

#define SAFEDELETE(ptr) if(ptr) { delete ptr; ptr = nullptr; }
#define SAFEDELETE_ARRAY(ptr) if(ptr) { delete[] ptr; ptr = nullptr; }
}


