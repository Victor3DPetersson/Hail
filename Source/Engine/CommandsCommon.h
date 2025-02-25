#pragma once

#include "Utility\Color.h"
#include "glm\vec2.hpp"
#include "glm\vec3.hpp"
#include "glm\vec4.hpp"

#include "glm\mat4x4.hpp"

#include "String.hpp"

#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\VectorOnStack\VectorOnStack.h"
#include "Transforms.h"
#include "Camera.h"
#include "EngineConstants.h"

namespace Hail
{
	struct DebugLineCommand
	{
		glm::vec3 pos1;
		glm::vec3 pos2;
		Color color1;
		Color color2;
		bool bLerpCommand;
		bool bIs2D;
		bool bIsAffectedBy2DCamera;
		bool bIsNormalized;
	};

	struct DepthTypeCounter2D
	{
		int m_layer{};
		uint16 m_textCounter{};
		uint16 m_spriteCounter{};
	};
}

