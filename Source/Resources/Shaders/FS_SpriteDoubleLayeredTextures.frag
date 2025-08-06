#include "shaderCommons.hs"

layout(binding = 0, set = 2) uniform sampler2D texSampler;
layout(binding = 1, set = 2) uniform sampler2D texSamplerOverlay;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 2) in float inCutoutThreshold;
//layout(location = 2) in uvec2 inInstanceIDEffectID;


void main() 
{
	vec2 timeOffset = vec2(g_constantVariables.totalTime_HorizonPosition.x * -1.0, g_constantVariables.totalTime_HorizonPosition.x * -1.0);
	vec4 color = texture(texSampler, vec2(inTexCoord.x, inTexCoord.y) + timeOffset).rgba;
	float alpha = texture(texSamplerOverlay, vec2(inTexCoord.x, inTexCoord.y)).a;
	if(alpha < inCutoutThreshold)
	{
		discard;
	}
	outColor = vec4(color * inColor);
	//outColor = uvec4(inTexCoord * 255.0, 0.0, 255.0);
}