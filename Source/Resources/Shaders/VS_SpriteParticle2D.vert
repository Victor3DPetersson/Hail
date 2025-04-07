#version 450

layout(binding = 0, set = 0, std140) uniform UniformBufferObject 
{
    uvec2 renderResolution;
    uvec2 screenResolution;
	uvec2 renderTargetRes;
	vec2 totalTime_HorizonPosition;
} g_constantVariables;

struct PointData
{
	vec2 pos;
	float scale;
	uint color;
};

layout(std140, set = 1, binding = 0) buffer PointDataBuffer 
{
   	PointData g_pointData[];
};

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out uint outColor;

const vec2 vertexBasePos[6] = 
{
	{-1.0, -1.0}, // BL
	{1.0, -1.0}, // BR
	{-1.0, 1.0}, // TL
	{-1.0, 1.0}, // TL
	{1.0, -1.0}, // BR
	{1.0, 1.0} // TR
};

const vec2 vertexUV[6] = 
{
	{-1.0, 1.0}, // TL , BR
	{1.0, 1.0}, // BR , BR
	{-1.0, -1.0}, // TL , TL
	{-1.0, -1.0}, // TL , TL
	{1.0, 1.0}, // BR , BR
	{1.0, -1.0} // BR , TL
};

void main()
{
	PointData pointData = g_pointData[gl_InstanceIndex];
	vec2 vertexPos = vertexBasePos[gl_VertexIndex];

	vec2 finalPosition;

	vec2 renderRes = vec2(g_constantVariables.renderTargetRes);

	vec2 scale = pointData.scale / renderRes;
	vec2 pivot = vec2(0.5, 0.5);

	vertexPos.x -= pivot.x;
	vertexPos.y += pivot.y;
	vertexPos *= scale;
	vertexPos += pointData.pos * 2.0 - 1.0;

	gl_Position = vec4(vertexPos.xy, 0.0, 1.0);
	outTexCoord = vertexUV[gl_VertexIndex];
	outColor = pointData.color;
}