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
	uvec4 textureSize_effectData_padding;
};

layout(std140, set = 1, binding = 1) buffer UIDataBuffer 
{
   	UIData uiInstanceData[];
};

uint BL = 0;
uint BR = 3;
uint TL = 1;
uint TR = 2;

const vec2 offsets[4] =
{
	{-1.0, 1.0}, //BL
	{-1.0, -1.0}, //TL
	{1.0, -1.0}, //TR
	{1.0, 1.0} //BR
};

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outColor;
//layout(location = 2) out uvec2 outInstanceIDEffectID;

void main() 
{
	UIData instanceData = uiInstanceData[PushConstants.instanceID_padding.x];
	//outInstanceIDEffectID.x = gl_InstanceIndex;
	//outInstanceIDEffectID.y = (instanceData.textureSize_effectData_padding.z >> 1)  & 1;
	bool scaleAfterRenderTargetSize = bool(instanceData.textureSize_effectData_padding.z & 1);
	vec2 renderRes = vec2(constantVariables.renderResolution);
	vec2 textureSize = vec2(instanceData.textureSize_effectData_padding.xy);
	vec2 finalScale = vec2(0);
	if(scaleAfterRenderTargetSize)
	{
		finalScale = instanceData.position_scale.zw;
	}
	else
	{
		vec2 relativeSizeToRes = (textureSize) / renderRes;
		finalScale = relativeSizeToRes * vec2(instanceData.position_scale.zw);
	}

	vec2 uvTL = instanceData.uvTL_BR.xy;
	vec2 uvBR = instanceData.uvTL_BR.zw;
	vec2 pivotPoint = instanceData.pivot_rotation_padding.xy * 2.0 - 1.0;
	vec2 finalUV = vec2(0);
	vec2 finalPosition = instanceData.position_scale.xy * 2.0 - 1.0;

	float rotation = instanceData.pivot_rotation_padding.z;
	float cs = cos(rotation);
	float sn = sin(rotation);
	vec2 offsetPos = vec2(0);
	vec2 rotatedPosition = vec2(0);

	if(inIndex == 0)
	{
	 	offsetPos = offsets[BL];
		finalUV.x = uvTL.x;
		finalUV.y = uvBR.y;
	}
	else if(inIndex == 1)
	{
		offsetPos = offsets[BR];
		finalUV.x = uvBR.x;
		finalUV.y = uvBR.y;
	}
	else if(inIndex == 2)
	{
		offsetPos = offsets[TL];
		finalUV.x = uvTL.x;
		finalUV.y = uvTL.y;
	}
	else if(inIndex == 3)
	{
		offsetPos = offsets[TL];
		finalUV.x = uvTL.x;
		finalUV.y = uvTL.y;
	}
	else if(inIndex == 4)
	{
		offsetPos = offsets[BR];
		finalUV.x = uvBR.x;
		finalUV.y = uvBR.y;
	}
	else
	{
		offsetPos = offsets[TR];
		finalUV.x = uvBR.x;
		finalUV.y = uvTL.y;
	}
	rotatedPosition.x = ((offsetPos.x + pivotPoint.x) * cs - (offsetPos.y + pivotPoint.y) * sn);
	rotatedPosition.y = ((offsetPos.x + pivotPoint.x) * sn + (offsetPos.y + pivotPoint.y) * cs);
	finalPosition.xy += rotatedPosition * finalScale;

	gl_Position = vec4(finalPosition, 0.01, 1.0);
	outTexCoord = vec2(finalUV.x, finalUV.y);
	outColor = instanceData.color;
}