#include "Shared_PCH.h"
#include "DebugLineHelpers.h"
#include "../Engine/RenderCommands.h"


namespace Hail
{
	void DrawLine2D(RenderCommandPool& poolToFill, glm::vec2 startPos, glm::vec2 endPos, glm::vec4 color1, glm::vec4 color2, bool drawPixelSpace)
	{
		const float normalizedSpaceModifier = drawPixelSpace ? 1.f : poolToFill.inverseHorizontalAspectRatio;
		RenderCommand_DebugLine line;
		line.pos1 = glm::vec3(drawPixelSpace ? startPos.x : startPos.x * normalizedSpaceModifier, startPos.y, 0.0f);
		line.pos2 = glm::vec3(drawPixelSpace ? endPos.x : endPos.x * normalizedSpaceModifier, endPos.y, 0.0f);
		line.color1 = color1;
		line.color2 = color2;
		line.bIs2D = true;
		line.bIsAffectedBy2DCamera = drawPixelSpace;
		line.bLerpCommand = true;
		poolToFill.debugLineCommands.Add(line);
	}
	void DrawLine2DPixelSpace(RenderCommandPool& poolToFill, glm::vec2 startPos, glm::vec2 endPos, glm::vec4 color1, glm::vec4 color2)
	{
		DrawLine2D(poolToFill, startPos, endPos, color1, color2, true);
	}
	void DrawLine2D(RenderCommandPool& poolToFill, glm::vec2 startPos, float rotationRadian, float length, glm::vec4 color1, glm::vec4 color2, bool drawPixelSpace)
	{
		const float normalizedSpaceModifier = drawPixelSpace ? 1.f : poolToFill.inverseHorizontalAspectRatio;
		RenderCommand_DebugLine line;
		line.pos1 = glm::vec3(startPos.x, startPos.y, 0.0f);
		const float x = sin(rotationRadian) * length * normalizedSpaceModifier;
		const float y = cos(rotationRadian) * -length;
		line.pos2 = glm::vec3(startPos.x + x, startPos.y + y, 0.0f);
		line.color1 = color1;
		line.color2 = color2;
		line.bIs2D = true;
		line.bLerpCommand = true;
		line.bIsAffectedBy2DCamera = drawPixelSpace;
		poolToFill.debugLineCommands.Add(line);
	}
	void DrawLine2DPixelSpace(RenderCommandPool& poolToFill, glm::vec2 startPos, float rotationRadian, float length, glm::vec4 color1, glm::vec4 color2)
	{
		DrawLine2D(poolToFill, startPos, rotationRadian, length, color1, color2, true);
	}
	void DrawBox2D(RenderCommandPool& poolToFill, glm::vec2 pos, glm::vec2 dimensions, glm::vec4 color, bool drawPixelSpace)
	{
		RenderCommand_DebugLine line;
		line.color1 = color;
		line.color2 = color;
		line.bIs2D = true;
		line.bLerpCommand = true;
		line.bIsAffectedBy2DCamera = drawPixelSpace;
		const glm::vec2 adjustedDimensions = glm::vec2(drawPixelSpace ? dimensions.x : dimensions.x * poolToFill.inverseHorizontalAspectRatio, dimensions.y);
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
	void DrawBox2DPixelSpace(RenderCommandPool& poolToFill, glm::vec2 pos, glm::vec2 dimensions, glm::vec4 color)
	{
		DrawBox2D(poolToFill, pos, dimensions, color, true);
	}
	void DrawBox2DMinMax(RenderCommandPool& poolToFill, glm::vec2 min, glm::vec2 max, glm::vec4 color, bool drawPixelSpace)
	{
		const glm::vec2 dimensions = max - min;
		const glm::vec2 halfDimensions = dimensions * 0.5f;
		DrawBox2D(poolToFill, min + halfDimensions, dimensions, color, drawPixelSpace);
	}

	void DrawBox2DMinMaxPixelSpace(RenderCommandPool& poolToFill, glm::vec2 min, glm::vec2 max, glm::vec4 color)
	{
		DrawBox2DMinMax(poolToFill, min, max, color, true);
	}

	void DrawCircle2D(RenderCommandPool& poolToFill, glm::vec2 pos, float radius, glm::vec4 color, bool drawPixelSpace)
	{
		RenderCommand_DebugLine line;
		line.color1 = color;
		line.color2 = color;
		line.bIs2D = true;
		line.bLerpCommand = true;
		line.bIsAffectedBy2DCamera = drawPixelSpace;

		const float dTheta = 2.0f * Math::PIf / 16.0;
		const float normalizedSpaceModifier = drawPixelSpace ? 1.f : poolToFill.inverseHorizontalAspectRatio;
		for (size_t i = 0; i <=  16; i++)
		{
			const float x1 = radius * cosf(i * dTheta) * normalizedSpaceModifier + pos.x;
			const float y1 = radius * sinf(i * dTheta) + pos.y;
			line.pos1 = glm::vec3(x1, y1, 0.0f);
			const size_t i2 = i == 16 ? 0 : i + 1;
			const float x2 = radius * cosf(i2 * dTheta) * normalizedSpaceModifier + pos.x;
			const float y2 = radius * sinf(i2 * dTheta) + pos.y;
			line.pos2 = glm::vec3(x2, y2, 0.0f);
			poolToFill.debugLineCommands.Add(line);
		}
	}

	void DrawCircle2DPixelSpace(RenderCommandPool& poolToFill, glm::vec2 pos, float radius, glm::vec4 color)
	{
		DrawCircle2D(poolToFill, pos, radius, color, true);
	}

}
