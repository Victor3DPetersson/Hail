#version 450
//Add to include file later
layout(binding = 0, set = 0, std140) uniform UniformBufferObject 
{
    uvec2 renderResolution;
    uvec2 screenResolution;
	vec2 totalTime_HorizonPosition;
} constantVariables;

layout(binding = 1, set = 1) uniform sampler2D texSampler;

layout(location = 0) out uvec4 outColor;
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;
//layout(location = 2) in uvec2 inInstanceIDEffectID;

void main() 
{
	vec4 color = texture(texSampler, vec2(inTexCoord.x, inTexCoord.y)).rgba;
	outColor = uvec4(color * inColor * 255.0);
	//outColor = uvec4(inTexCoord * 255.0, 0.0, 255.0);
}