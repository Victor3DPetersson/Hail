#version 450
#extension GL_EXT_shader_atomic_float : require

const float PIf = 3.1415926;
const float PIfHalf = PIf / 2.0;
const float PI2f = PIf * 2.0;
const float GoldenAngle = PIf * (0.763932);
const float EpsilonF = 0.00001;

layout(binding = 0, set = 0, std140) uniform UniformBufferObject 
{
    uvec2 renderResolution;
    uvec2 screenResolution;
	uvec2 renderTargetRes;
	vec2 totalTime_HorizonPosition;
	vec2 deltaTime_cameraZoom;
	vec2 cameraPos; // normalized
	vec2 mousePos; // normalized
	vec2 mouseRMBLMBDeltas;

	vec2 playerPosition; // TEMP;
	vec2 padding;
} g_constantVariables;

layout(set = 0, binding = 2) uniform sampler g_samplerLinear;
layout(set = 0, binding = 3) uniform sampler g_samplerBilinear;
layout(set = 0, binding = 4) uniform sampler g_samplerBilinearClampBorder;
