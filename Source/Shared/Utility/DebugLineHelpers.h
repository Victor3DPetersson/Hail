#pragma once
#include "Types.h"


namespace Hail
{
	struct RenderCommandPool;
	//Normalized space
	void DrawLine2D(RenderCommandPool& poolToFill, glm::vec2 startPos, glm::vec2 endPos, glm::vec4 color1 = Vec4One, glm::vec4 color2 = Vec4One);
	//Normalized space, compensates for the render targets aspect ratio
	void DrawLine2D(RenderCommandPool& poolToFill, glm::vec2 startPos, float rotationRadian, float length, glm::vec4 color1 = Vec4One, glm::vec4 color2 = Vec4One);
	//Normalized space, The Box functions compensates for the render targets aspect ratio
	void DrawBox2D(RenderCommandPool& poolToFill, glm::vec2 pos, glm::vec2 dimensions, glm::vec4 color = Vec4One);
	void DrawBox2DMinMax(RenderCommandPool& poolToFill, glm::vec2 min, glm::vec2 max, glm::vec4 color = Vec4One);
	//Normalized space, The Circle function compensates for the render targets aspect ratio
	void DrawCircle2D(RenderCommandPool& poolToFill, glm::vec2 pos, float radius, glm::vec4 color = Vec4One);
}
