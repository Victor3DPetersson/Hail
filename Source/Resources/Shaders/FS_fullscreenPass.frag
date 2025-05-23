#version 450
//Add to include file later
layout(binding = 0, set = 0, std140) uniform UniformBufferObject 
{
    uvec2 renderResolution; // Actual render resolution on the final window
    uvec2 screenResolution;
	uvec2 renderTargetRes; // Main rendertargets resolution
	vec2 totalTime_HorizonPosition;
} constantVariables;

layout(binding = 0, set = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragTexCoord;

void main() 
{

	float renderTexelSizeX = 1.0 / float(constantVariables.renderResolution.x);
	float renderTexelSizeY = 1.0 / float(constantVariables.renderResolution.y);
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
		float remappedXCoord = min(float(xIndex - xHalfOffset) / float(constantVariables.renderResolution.x), 1.0 - renderTexelSizeX * 0.5);
		float remappedYCoord = min((1.0 - float(yIndex - yHalfOffset) / float(constantVariables.renderResolution.y)) + renderTexelSizeY * 0.5, 1.0 - renderTexelSizeY * 0.5);
		vec3 sampledColor = texture(texSampler, vec2(remappedXCoord, remappedYCoord)).rgb;
		vec3 color = vec3(sampledColor);
		//vec3 color = vec3(remappedXCoord, remappedYCoord, 1.0);
		outColor = vec4(color, 1.0);
	}
}