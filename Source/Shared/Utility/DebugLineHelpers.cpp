#include "Shared_PCH.h"
#include "DebugLineHelpers.h"
#include "../Engine/Interface/GameCommands.h"


namespace Hail
{
	void DrawLine2D(ApplicationCommandPool& poolToFill, glm::vec2 startPos, glm::vec2 endPos, bool bAffectedBy2DCamera, glm::vec4 color1, glm::vec4 color2, bool drawPixelSpace)
	{
		DebugLineCommand line;
		line.pos1 = glm::vec3(startPos.x, startPos.y, 0.0f);
		line.pos2 = glm::vec3(endPos.x, endPos.y, 0.0f);
		line.color1 = color1;
		line.color2 = color2;
		line.bIs2D = true;
		line.bIsAffectedBy2DCamera = bAffectedBy2DCamera;
		line.bLerpCommand = true;
		line.bIsNormalized = !drawPixelSpace;
		poolToFill.AddDebugLine(line);
	}

	void DrawLine2DPixelSpace(ApplicationCommandPool& poolToFill, glm::vec2 startPos, glm::vec2 endPos, bool bAffectedBy2DCamera, glm::vec4 color1, glm::vec4 color2)
	{
		DrawLine2D(poolToFill, startPos, endPos, bAffectedBy2DCamera, color1, color2, true);
	}

	void DrawLine2D(ApplicationCommandPool& poolToFill, glm::vec2 startPos, float rotationRadian, float length, bool bAffectedBy2DCamera, glm::vec4 color1, glm::vec4 color2, bool drawPixelSpace)
	{
		const float normalizedSpaceModifier = drawPixelSpace ? 1.f : poolToFill.inverseHorizontalAspectRatio;
		DebugLineCommand line;
		line.pos1 = glm::vec3(startPos.x, startPos.y, 0.0f);
		const float x = sin(rotationRadian) * length * normalizedSpaceModifier;
		const float y = cos(rotationRadian) * -length;
		line.pos2 = glm::vec3(startPos.x + x, startPos.y + y, 0.0f);
		line.color1 = color1;
		line.color2 = color2;
		line.bIs2D = true;
		line.bLerpCommand = true;
		line.bIsAffectedBy2DCamera = bAffectedBy2DCamera;
		line.bIsNormalized = !drawPixelSpace;
		poolToFill.AddDebugLine(line);
	}

	void DrawLine2DPixelSpace(ApplicationCommandPool& poolToFill, glm::vec2 startPos, float rotationRadian, float length, bool bAffectedBy2DCamera, glm::vec4 color1, glm::vec4 color2)
	{
		DrawLine2D(poolToFill, startPos, rotationRadian, length, bAffectedBy2DCamera, color1, color2, true);
	}

	void DrawBox2D(ApplicationCommandPool& poolToFill, glm::vec2 pos, glm::vec2 dimensions, bool bAffectedBy2DCamera, glm::vec4 color, bool drawPixelSpace)
	{
		DebugLineCommand line;
		line.color1 = color;
		line.color2 = color;
		line.bIs2D = true;
		line.bLerpCommand = true;
		line.bIsAffectedBy2DCamera = bAffectedBy2DCamera;
		line.bIsNormalized = !drawPixelSpace;
		const glm::vec2 adjustedDimensions = glm::vec2(drawPixelSpace ? dimensions.x : dimensions.x * poolToFill.inverseHorizontalAspectRatio, dimensions.y);
		//Top
		line.pos1 = glm::vec3(pos.x - adjustedDimensions.x, pos.y + adjustedDimensions.y, 0.0);
		line.pos2 = glm::vec3(pos.x + adjustedDimensions.x, pos.y + adjustedDimensions.y, 0.0);
		poolToFill.AddDebugLine(line);
		//Right
		line.pos1 = glm::vec3(pos.x + adjustedDimensions.x, pos.y + adjustedDimensions.y, 0.0);
		line.pos2 = glm::vec3(pos.x + adjustedDimensions.x, pos.y - adjustedDimensions.y, 0.0);
		poolToFill.AddDebugLine(line);
		//Bottom
		line.pos1 = glm::vec3(pos.x - adjustedDimensions.x, pos.y - adjustedDimensions.y, 0.0);
		line.pos2 = glm::vec3(pos.x + adjustedDimensions.x, pos.y - adjustedDimensions.y, 0.0);
		poolToFill.AddDebugLine(line);
		//Left
		line.pos1 = glm::vec3(pos.x - adjustedDimensions.x, pos.y + adjustedDimensions.y, 0.0);
		line.pos2 = glm::vec3(pos.x - adjustedDimensions.x, pos.y - adjustedDimensions.y, 0.0);
		poolToFill.AddDebugLine(line);
	}

	void DrawBox2DPixelSpace(ApplicationCommandPool& poolToFill, glm::vec2 pos, glm::vec2 dimensions, bool bAffectedBy2DCamera, glm::vec4 color)
	{
		DrawBox2D(poolToFill, pos, dimensions, bAffectedBy2DCamera, color, true);
	}

	void DrawBox2DMinMax(ApplicationCommandPool& poolToFill, glm::vec2 min, glm::vec2 max, bool bAffectedBy2DCamera, glm::vec4 color, bool drawPixelSpace)
	{
		const glm::vec2 dimensions = max - min;
		const glm::vec2 halfDimensions = dimensions * 0.5f;
		DrawBox2D(poolToFill, min + halfDimensions, dimensions, bAffectedBy2DCamera, color, drawPixelSpace);
	}

	void DrawBox2DMinMaxPixelSpace(ApplicationCommandPool& poolToFill, glm::vec2 min, glm::vec2 max, bool bAffectedBy2DCamera, glm::vec4 color)
	{
		DrawBox2DMinMax(poolToFill, min, max, bAffectedBy2DCamera, color, true);
	}

	void DrawCircle2D(ApplicationCommandPool& poolToFill, glm::vec2 pos, float radius, bool bAffectedBy2DCamera, glm::vec4 color, bool drawPixelSpace)
	{
		DebugLineCommand line;
		line.color1 = color;
		line.color2 = color;
		line.bIs2D = true;
		line.bLerpCommand = true;
		line.bIsAffectedBy2DCamera = bAffectedBy2DCamera;
		line.bIsNormalized = !drawPixelSpace;

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
			poolToFill.AddDebugLine(line);
		}
	}

	void DrawCircle2DPixelSpace(ApplicationCommandPool& poolToFill, glm::vec2 pos, float radius, bool bAffectedBy2DCamera, glm::vec4 color)
	{
		DrawCircle2D(poolToFill, pos, radius, bAffectedBy2DCamera, color, true);
	}

}
