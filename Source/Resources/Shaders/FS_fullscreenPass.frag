#version 450
//Add to include file later
layout(binding = 0, set = 0, std140) uniform UniformBufferObject 
{
    uvec2 renderResolution;
    uvec2 screenResolution;
	float totalTime;
} constantVariables;

layout(binding = 1, set = 1) uniform usampler2D texSampler;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragTexCoord;

void main() 
{

	uint xHalfOffset = (constantVariables.screenResolution.x - constantVariables.renderResolution.x) / 2;
	uint yHalfOffset = (constantVariables.screenResolution.y - constantVariables.renderResolution.y) / 2;

	uint xIndex = uint(floor(fragTexCoord.x * float(constantVariables.screenResolution.x))); 
	uint yIndex = uint(floor(fragTexCoord.y * float(constantVariables.screenResolution.y))); 

    //outColor = texture(texSampler, vec2(1.0-fragTexCoord.x, fragTexCoord.y)).rgba;
	//if outside of main render area
	if(xIndex < xHalfOffset || xIndex > constantVariables.renderResolution.x + xHalfOffset ||
	yIndex < yHalfOffset || yIndex > constantVariables.renderResolution.y + yHalfOffset)
	{	
		outColor = vec4(0.41, 0.42, 0.51, 1.0);
	}
	else
	{
		float ratioX = constantVariables.screenResolution.x / constantVariables.screenResolution.y;
		float remappedXCoord = float(xIndex - xHalfOffset) / float(constantVariables.renderResolution.x);
		float remappedYCoord = 1.0 - float(yIndex - yHalfOffset) / float(constantVariables.renderResolution.y);
		uvec3 sampledColor = texture(texSampler, vec2(remappedXCoord, remappedYCoord)).rgb;
		vec3 color = vec3(sampledColor) / 255.0;
		//vec3 color = vec3(remappedXCoord, remappedYCoord, 1.0);
		outColor = vec4(color, 1.0);
	}
}