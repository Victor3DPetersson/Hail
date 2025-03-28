#version 450
//Add to include file later
layout(binding = 0, set = 0, std140) uniform UniformBufferObject 
{
    uvec2 renderResolution;
    uvec2 screenResolution;
	uvec2 renderTargetRes;
	vec2 totalTime_HorizonPosition;
} constantVariables;

layout(binding = 1, set = 2) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 2) in float inCutoutThreshold;
//layout(location = 2) in uvec2 inInstanceIDEffectID;

void main() 
{
	vec4 color = texture(texSampler, vec2(inTexCoord.x, inTexCoord.y)).rgba;
	if(color.a < inCutoutThreshold)
	{
		discard;
	}
	outColor = vec4(color * inColor);
	//outColor = uvec4(inTexCoord * 255.0, 0.0, 255.0);
}