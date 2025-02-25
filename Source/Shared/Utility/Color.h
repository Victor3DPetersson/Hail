#pragma once

#include "Types.h"
#include "glm\glm.hpp"

namespace Hail
{
	class Color
	{
	public:
		Color();
		Color(glm::vec3 color, float alpha);
		Color(glm::vec4 colorAndAlpha);
		Color(uint32 packedColor);

		Color& operator=(uint32 color);
		Color& operator=(glm::vec3 color);
		Color& operator=(glm::vec4 colorAndAlpha);

		uint32 GetColorPacked() const;
		glm::vec3 GetColorVec3() const;
		glm::vec4 GetColorWithAlpha() const;

		void SetR(float colorValue);
		void SetG(float colorValue);
		void SetB(float colorValue);
		void SetA(float colorValue);

		static Color Lerp(const Color& color1, const Color& color2, float T);

	private:
		uint32 m_colorValue;
	};
}

