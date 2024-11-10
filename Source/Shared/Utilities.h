#pragma once
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

	inline bool IsSystemBigEndian()
	{
		union {
			uint32_t i;
			char c[4];
		} endianInt = { 0x01020304 };

		return endianInt.c[0] == 1;
	}

	template <typename T>
	void SwapEndian(T& u)
	{
		static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

		union
		{
			T u;
			unsigned char u8[sizeof(T)];
		} source, dest;

		source.u = u;

		for (size_t k = 0; k < sizeof(T); k++)
			dest.u8[k] = source.u8[sizeof(T) - k - 1];

		u = dest.u;
	}
	
	static bool IsBitSet(uint8 byte, int bitIndex)
	{
		return ((byte >> bitIndex) & 1) == 1;
	}

	static bool IsBitSet(uint16 byte, int bitIndex)
	{
		return ((byte >> bitIndex) & 1) == 1;
	}

	static bool IsBitSet(uint32 byte, int bitIndex)
	{
		return ((byte >> bitIndex) & 1) == 1;
	}

	static bool IsBitSet(uint64 byte, int bitIndex)
	{
		return ((byte >> bitIndex) & 1) == 1;
	}
}


