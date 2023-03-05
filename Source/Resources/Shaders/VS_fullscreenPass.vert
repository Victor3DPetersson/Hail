#version 450

layout(location = 0) in uint inIndex;

layout(location = 0) out vec2 outTexCoord;

vec4 pos[3] =
	{
		vec4(-1.0, -1.0, 0.0, 1.0),
		vec4(-1.0, 3.0, 0.0, 1.0),
		vec4(3.0, -1.0, 0.0, 1.0)
	};
	
	vec2 uv[3] =
	{
		vec2(0.0, 1.0),
		vec2(0.0, -1.0),
		vec2(2.0, 1.0)
	};

void main() 
{
	gl_Position = pos[inIndex];
	outTexCoord = uv[inIndex];
}