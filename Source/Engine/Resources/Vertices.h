#pragma once

#include "glm\vec2.hpp"
#include "glm\vec3.hpp"
#include "glm\vec4.hpp"

namespace Hail
{
	enum class VERTEX_TYPES : uint32_t
	{
		SPRITE,
		MODEL,
		ANIMATION,
		PARTICLE
	};

	struct VertexSprite
	{
		uint32_t index;
	};

	struct VertexModel
	{
		glm::vec3 pos = { 0.0, 0.0, 0.0 };
		glm::vec3 norm = { 0.0, 0.0, 0.0 };
		glm::vec3 tangent = { 0.0, 0.0, 0.0 };
		glm::vec3 biTangent = { 0.0, 0.0, 0.0 };
		glm::vec4 color = { 1.0, 1.0, 1.0, 1.0 };
		glm::vec2 texCoord1 = { 0.0, 0.0 };
		glm::vec2 texCoord2 = { 0.0, 0.0 };
	};

	struct VertexAnimation
	{
		glm::vec3 pos = { 0.0, 0.0, 0.0 };
		glm::vec3 norm = { 0.0, 0.0, 0.0 };
		glm::vec3 tangent = { 0.0, 0.0, 0.0 };
		glm::vec3 biTangent = { 0.0, 0.0, 0.0 };
		glm::vec4 color = { 1.0, 1.0, 1.0, 1.0 };
		glm::vec2 texCoord1 = { 0.0, 0.0 };
		glm::vec2 texCoord2 = { 0.0, 0.0 };
		glm::vec4 weigths = { 0.0, 0.0, 0.0, 0.0 };
		glm::u32vec4 indices = { 0, 0, 0, 0 };
	};
}

