#version 450

// TODO: create a utility header for shaders with common operations
vec4 ColorFromPackedColor(uint packedColor)
{
    vec4 returnColor;
    returnColor.x = float((packedColor >> 24) & 0xff) / 255.0;
    returnColor.y = float((packedColor >> 16) & 0xff) / 255.0;
    returnColor.z = float((packedColor >> 8) & 0xff) / 255.0;
    returnColor.w = float((packedColor) & 0xff) / 255.0;
    return returnColor;
}

layout(binding = 0, set = 0, std140) uniform UniformBufferObject 
{
    uvec2 renderResolution;
    uvec2 screenResolution;
	uvec2 renderTargetRes;
	vec2 totalTime_HorizonPosition;
} constantVariables;

layout( push_constant ) uniform constants
{
	uvec4 instanceID_padding; // uint uvec3 padding
} PushConstants;

layout(location = 0) in uint inIndex;

struct Instance2D
{
	vec4 position_scale;
	float rotation;
	uint packedColor;
	uint index_materialIndex_flags; // used for sorting
	uint dataIndex; // index to the sprite specific data
};

layout(std140, set = 1, binding = 0) buffer InstanceBuffer 
{
   	Instance2D g_instances[];
};

struct SpriteData
{
	vec4 uvTL_BR; // f2, f2
	vec4 pivot_sizeMultiplier; // f2, f2
	vec4 cutoutThreshold_padding; // f, f3
	vec4 padding; // f, f3
};

layout(std140, set = 1, binding = 1) buffer SpriteDataBuffer 
{
   	SpriteData g_spriteData[];
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
layout(location = 2) out float outCutoutThreshold;
//layout(location = 2) out uvec2 outInstanceIDEffectID;

void main() 
{
	uint instanceIndex = PushConstants.instanceID_padding.x + gl_InstanceIndex;
	Instance2D instanceData = g_instances[instanceIndex];
	SpriteData spriteData = g_spriteData[instanceData.dataIndex];
	//UIData instanceData = uiInstanceData[PushConstants.instanceID_padding.x];

	vec2 renderRes = vec2(constantVariables.renderResolution);
	vec2 finalScale = vec2(instanceData.position_scale.zw);

	float ratio = renderRes.y / renderRes.x;
	
	vec2 uvTL = spriteData.uvTL_BR.xy;
	vec2 uvBR = spriteData.uvTL_BR.zw;
	vec2 pivotPoint = spriteData.pivot_sizeMultiplier.xy * 2.0 - 1.0;
	vec2 finalUV = vec2(0);
	float rotation = instanceData.rotation;

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
	
	vertexPos.xy *= (finalScale * spriteData.pivot_sizeMultiplier.zw);
	float cs = cos(rotation);
	float sn = sin(rotation);

	mat2 rotationMatrix = mat2(cs, -sn, sn, cs);
	vertexPos.xy = vertexPos.xy * rotationMatrix;
	vertexPos.x *= ratio;
	vertexPos.xy += instanceData.position_scale.xy * 2.0 - 1.0; // add input position

	gl_Position = vec4(vertexPos.xy, 0.00, 1.0);
	outTexCoord = vec2(finalUV.x, finalUV.y);
	outColor = ColorFromPackedColor(instanceData.packedColor);
	outCutoutThreshold = spriteData.cutoutThreshold_padding.x;
}