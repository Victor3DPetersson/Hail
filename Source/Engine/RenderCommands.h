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
#include "EngineConstants.h"

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
		uint16_t index;
		bool lerpCommand;
	};

	struct RenderCommand_Sprite
	{
		Transform2D transform;
		glm::vec4 uvTR_BL = { 0.0, 0.0, 1.0, 1.0 };
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec2 pivot = { 0.5f, 0.5f };
		uint32_t materialInstanceID;
		uint16_t index;
		bool lerpCommand = true;
		bool sizeRelativeToRenderTarget = false;
	};

	struct RenderCommand_Text
	{
		Transform2D transform;
		glm::vec3 color;
		String256 text;
		uint32_t materialInstanceID;
		uint16_t index;
		bool lerpCommand;
	};

	struct RenderCommand_Mesh
	{
		Transform3D transform;
		glm::vec3 color;
		uint32_t meshID = 0;
		uint32_t materialInstanceID = 0;
		uint16_t index = 0;
		bool lerpCommand = true;
	};


	struct RenderCommandPool
	{
		Camera renderCamera;
		VectorOnStack<RenderCommand_DebugLine, 1024, false> debugLineCommands;
		VectorOnStack<RenderCommand_Sprite, MAX_NUMBER_OF_SPRITES, false> spriteCommands;
		VectorOnStack<RenderCommand_Text, 1024, false> textCommands;
		VectorOnStack<RenderCommand_Mesh, 1024, false> meshCommands;
	};

}

