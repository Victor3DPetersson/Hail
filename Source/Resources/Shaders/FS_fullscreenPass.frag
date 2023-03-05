#version 450
//Add to include file later
layout(binding = 0, std140) uniform UniformBufferObject 
{
    uvec2 screenResolution;
    uvec2 renderResolution;
	float totalTime;
} constantVariables;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragTexCoord;

void main() 
{

	uint xHalfOffset = (constantVariables.screenResolution.x - constantVariables.renderResolution.x) / 2;
	uint yHalfOffset = (constantVariables.screenResolution.y - constantVariables.renderResolution.y) / 2;

	uint xIndex = uint(floor(fragTexCoord.x * constantVariables.screenResolution.x)); 
	uint yIndex = uint(floor(fragTexCoord.y * constantVariables.screenResolution.y)); 

    outColor = texture(texSampler, vec2(1.0-fragTexCoord.x, fragTexCoord.y)).rgba;

	//if outside of main render area
	if(xIndex < xHalfOffset || xIndex > constantVariables.renderResolution.x + xHalfOffset ||
	yIndex < yHalfOffset || yIndex > constantVariables.renderResolution.y + yHalfOffset)
	{
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		float ratioX = constantVariables.screenResolution.x / constantVariables.screenResolution.y;
		float remappedXCoord = (xIndex - xHalfOffset) / constantVariables.renderResolution.x;
		float remappedYCoord = (yIndex - yHalfOffset) / constantVariables.renderResolution.y;
		vec3 color = texture(texSampler, vec2(1.0-remappedXCoord, remappedYCoord)).rgb;
		outColor = vec4(color, 1.0);
	}
}