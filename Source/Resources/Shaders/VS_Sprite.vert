#version 450

layout( push_constant ) uniform constants
{
	uvec4 instanceID_padding; // uint uvec3 padding
} PushConstants;

layout(location = 0) in uint inIndex;

layout(binding = 0, set = 0, std140) uniform UniformBufferObject 
{
    uvec2 renderResolution;
    uvec2 screenResolution;
	vec2 totalTime_HorizonPosition;
} constantVariables;

struct UIData
{
	vec4 position_scale;
	vec4 uvTL_BR;
	vec4 color;
	vec4 pivot_rotation_padding; //vec2 float padd
	vec4 scaleMultiplier_effectData_padding;
};

layout(std140, set = 1, binding = 1) buffer UIDataBuffer 
{
   	UIData uiInstanceData[];
};

uint BL = 0;
uint BR = 3;
uint TL = 1;
uint TR = 2;

//TODO: set these on the vertices instead
const vec2 offsets[4] =
{
	{-1.0, -1.0}, //BL
	{-1.0, 1.0}, //TL
	{1.0, 1.0}, //TR
	{1.0, -1.0} //BR
};

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outColor;
//layout(location = 2) out uvec2 outInstanceIDEffectID;

void main() 
{
	UIData instanceData = uiInstanceData[PushConstants.instanceID_padding.x];

	vec2 renderRes = vec2(constantVariables.renderResolution);
	vec2 finalScale = vec2(instanceData.position_scale.zw);

	float ratio = renderRes.y / renderRes.x;
	
	vec2 uvTL = instanceData.uvTL_BR.xy;
	vec2 uvBR = instanceData.uvTL_BR.zw;
	vec2 pivotPoint = instanceData.pivot_rotation_padding.xy * 2.0 - 1.0;
	vec2 finalUV = vec2(0);
	float rotation = instanceData.pivot_rotation_padding.z;
	float cs = cos(rotation);
	float sn = sin(rotation);
	//TODO: set these on the vertices instead
	vec4 vertexPos = vec4(0.0 ,0.0, 0.0, 1.0);
	if(inIndex == 0)
	{
	 	vertexPos.xy = offsets[BL];
		finalUV.x = uvTL.x;
		finalUV.y = uvBR.y;
	}
	else if(inIndex == 1)
	{
		vertexPos.xy = offsets[BR];
		finalUV.x = uvBR.x;
		finalUV.y = uvBR.y;
	}
	else if(inIndex == 2)
	{
		vertexPos.xy = offsets[TL];
		finalUV.x = uvTL.x;
		finalUV.y = uvTL.y;
	}
	else if(inIndex == 3)
	{
		vertexPos.xy = offsets[TL];
		finalUV.x = uvTL.x;
		finalUV.y = uvTL.y;
	}
	else if(inIndex == 4)
	{
		vertexPos.xy = offsets[BR];
		finalUV.x = uvBR.x;
		finalUV.y = uvBR.y;
	}
	else
	{
		vertexPos.xy = offsets[TR];
		finalUV.x = uvBR.x;
		finalUV.y = uvTL.y;
	}

	vertexPos.x -= pivotPoint.x;
	vertexPos.y += pivotPoint.y;
	
	vertexPos.xy *= finalScale;
	
	mat2 theRotation = mat2(cs, -sn, sn, cs);
	vertexPos.xy = vertexPos.xy * theRotation;
	vertexPos.x *= ratio;
	vertexPos.xy += instanceData.position_scale.xy * 2.0 - 1.0; // add input position

	gl_Position = vec4(vertexPos.xy, 0.01, 1.0);
	outTexCoord = vec2(finalUV.x, finalUV.y);
	outColor = instanceData.color;
}