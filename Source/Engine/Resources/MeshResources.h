#pragma once
#include "Vertices.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "glm\vec3.hpp"
namespace Hail
{
	struct Mesh
	{
		GrowingArray<VertexModel> vertices;
		GrowingArray<uint32_t> indices;
		String64 material;
		glm::vec3 min, max;
	};

}

