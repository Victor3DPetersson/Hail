#include "Shared_PCH.h"
#include "DebugLineHelpers.h"
#include "../Engine/RenderCommands.h"


namespace Hail
{
	void DrawLine2D(RenderCommandPool& poolToFill, glm::vec2 startPos, glm::vec2 endPos, glm::vec4 color1, glm::vec4 color2)
	{
		RenderCommand_DebugLine line;
		line.pos1 = glm::vec3(startPos.x, startPos.y, 0.0f);
		line.pos2 = glm::vec3(endPos.x, endPos.y, 0.0f);
		line.color1 = color1;
		line.color2 = color2;
		line.is2D = true;
		line.lerpCommand = true;
		poolToFill.debugLineCommands.Add(line);
	}
	void DrawLine2D(RenderCommandPool& poolToFill, glm::vec2 startPos, float rotationRadian, float length, glm::vec4 color1, glm::vec4 color2)
	{
		RenderCommand_DebugLine line;
		line.pos1 = glm::vec3(startPos.x, startPos.y, 0.0f);
		const float x = sin(rotationRadian) * length * poolToFill.inverseHorizontalAspectRatio;
		const float y = cos(rotationRadian) * -length;
		line.pos2 = glm::vec3(startPos.x + x, startPos.y + y, 0.0f);
		line.color1 = color1;
		line.color2 = color2;
		line.is2D = true;
		line.lerpCommand = true;
		poolToFill.debugLineCommands.Add(line);
	}
	void DrawBox2D(RenderCommandPool& poolToFill, glm::vec2 pos, glm::vec2 dimensions, glm::vec4 color)
	{
		RenderCommand_DebugLine line;
		line.color1 = color;
		line.color2 = color;
		line.is2D = true;
		line.lerpCommand = true;
		const glm::vec2 adjustedDimensions = glm::vec2(dimensions.x * poolToFill.inverseHorizontalAspectRatio, dimensions.y);
		//Top
		line.pos1 = glm::vec3(pos.x - adjustedDimensions.x, pos.y + adjustedDimensions.y, 0.0);
		line.pos2 = glm::vec3(pos.x + adjustedDimensions.x, pos.y + adjustedDimensions.y, 0.0);
		poolToFill.debugLineCommands.Add(line);
		//Right
		line.pos1 = glm::vec3(pos.x + adjustedDimensions.x, pos.y + adjustedDimensions.y, 0.0);
		line.pos2 = glm::vec3(pos.x + adjustedDimensions.x, pos.y - adjustedDimensions.y, 0.0);
		poolToFill.debugLineCommands.Add(line);
		//Bottom
		line.pos1 = glm::vec3(pos.x - adjustedDimensions.x, pos.y - adjustedDimensions.y, 0.0);
		line.pos2 = glm::vec3(pos.x + adjustedDimensions.x, pos.y - adjustedDimensions.y, 0.0);
		poolToFill.debugLineCommands.Add(line);
		//Left
		line.pos1 = glm::vec3(pos.x - adjustedDimensions.x, pos.y + adjustedDimensions.y, 0.0);
		line.pos2 = glm::vec3(pos.x - adjustedDimensions.x, pos.y - adjustedDimensions.y, 0.0);
		poolToFill.debugLineCommands.Add(line);
	}
	void DrawBox2DMinMax(RenderCommandPool& poolToFill, glm::vec2 min, glm::vec2 max, glm::vec4 color)
	{
		const glm::vec2 dimensions = max - min;
		const glm::vec2 halfDimensions = dimensions * 0.5f;
		DrawBox2D(poolToFill, min + halfDimensions, dimensions, color);
	}

	void DrawCircle2D(RenderCommandPool& poolToFill, glm::vec2 pos, float radius, glm::vec4 color)
	{
		RenderCommand_DebugLine line;
		line.color1 = color;
		line.color2 = color;
		line.is2D = true;
		line.lerpCommand = true;

		const float dTheta = 2.0f * Math::PIf / 16.0;
		for (size_t i = 0; i <=  16; i++)
		{
			const float x1 = radius * cosf(i * dTheta) * poolToFill.inverseHorizontalAspectRatio + pos.x;
			const float y1 = radius * sinf(i * dTheta) + pos.y;
			line.pos1 = glm::vec3(x1, y1, 0.0f);
			const size_t i2 = i == 16 ? 0 : i + 1;
			const float x2 = radius * cosf(i2 * dTheta) * poolToFill.inverseHorizontalAspectRatio + pos.x;
			const float y2 = radius * sinf(i2 * dTheta) + pos.y;
			line.pos2 = glm::vec3(x2, y2, 0.0f);
			poolToFill.debugLineCommands.Add(line);
		}

	}

}
