#pragma once
#include "glm\vec2.hpp"
#include "glm\vec3.hpp"
#include "glm\vec4.hpp"

#include "glm\mat4x4.hpp"

#include "String.hpp"

#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\VectonOnStack\VectorOnStack.h"
#include "Transforms.h"
#include "Camera.h"

namespace Hail
{

	struct RenderCommand_RenderCamera
	{

	};

	struct RenderCommand_DebugLine
	{
		glm::vec3 pos1;
		glm::vec3 pos2;
		glm::vec3 color;
	};

	struct RenderCommand_Sprite
	{
		Transform2D transform;
		glm::vec4 uv;
		glm::vec3 color;
		uint32_t materialInstanceID;
	};

	struct RenderCommand_Text
	{
		Transform2D transform;
		glm::vec3 color;
		String256 text;
		uint32_t materialInstanceID;
	};

	struct RenderCommand_Mesh
	{
		Transform3D transform;
		glm::vec3 color;
		uint32_t meshID;
		uint32_t materialInstanceID;
	};


	struct RenderCommandPool
	{
		Camera renderCamera;
		VectorOnStack<RenderCommand_DebugLine, 1024, false> debugLineCommands;
		VectorOnStack<RenderCommand_Sprite, 1024, false> spriteCommands;
		VectorOnStack<RenderCommand_Text, 1024, false> textCommands;
		VectorOnStack<RenderCommand_Mesh, 1024, false> meshCommands;
	};

}

