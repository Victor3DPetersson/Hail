#pragma once
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

	struct RenderCommand_RenderCamera
	{

	};

	struct RenderCommand_DebugLine
	{
		glm::vec3 pos1;
		glm::vec3 pos2;
		glm::vec4 color1; 
		glm::vec4 color2;
		bool bLerpCommand;
		bool bIs2D;
		bool bIsAffectedBy2DCamera;
		bool bIsNormalized;
	};

	struct RenderCommand_Sprite
	{
		Transform2D transform;
		glm::vec4 uvTR_BL = { 0.0, 0.0, 1.0, 1.0 };
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec2 pivot = { 0.5f, 0.5f };
		uint32_t materialInstanceID = MAX_UINT;
		uint16_t index;
		bool bLerpCommand = true;
		// true the scale will be in normalized size, so a size of 0.1 will always stay 0.1 of the screen percentage
		// false the scale will be of the texture size on the render target, so the texture will be scaled by the scale, 
		// false best used with UI or elements that should not scale with screen resolution
		bool bSizeRelativeToRenderTarget = true;
		bool bIsAffectedBy2DCamera = true;
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
		float horizontalAspectRatio{}; // if this is 16 / 9 = 1.77
		float inverseHorizontalAspectRatio{};
		Camera camera3D;
		Camera2D camera2D;
		VectorOnStack<RenderCommand_DebugLine, MAX_NUMBER_OF_DEBUG_LINES / 2, false> debugLineCommands;
		VectorOnStack<RenderCommand_Sprite, MAX_NUMBER_OF_SPRITES, false> spriteCommands;
		VectorOnStack<RenderCommand_Text, 1024, false> textCommands;
		VectorOnStack<RenderCommand_Mesh, 1024, false> meshCommands;
	};

}

