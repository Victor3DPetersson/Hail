#include "shaderCommons.hs"

layout(binding = 1, set = 2) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 2) in float inCutoutThreshold;

const vec3 skyColorTable[5] = {
    {191.0 / 255.0, 214.0 / 255.0, 214 / 255.0}, // Top
    {158.0 / 255.0, 174.0 / 255.0, 188.0 / 255.0},
    {117.0 / 255.0, 122.0 / 255.0, 142.0 / 255.0},
    {75.0 / 255.0, 69.0 / 255.0, 95.0 / 255.0},
    {57.0 / 255.0, 47.0 / 255.0, 75.0 / 255.0}  // Bottom
};

const mat4x4 bayerMatrix4x4 = mat4x4(
    0.0,  8.0,  2.0, 10.0,
    12.0, 4.0,  14.0, 6.0,
    3.0,  11.0, 1.0, 9.0,
    15.0, 7.0,  13.0, 5.0
) / 16.0;

void main() 
{
    vec2 pixelCoord = vec2(inTexCoord.x, inTexCoord.y) * vec2(g_constantVariables.renderTargetRes);
    vec2 cameraPixelPos = g_constantVariables.cameraPos * vec2(g_constantVariables.renderTargetRes);
    vec2 pixelCameraPos = vec2(inTexCoord.x, inTexCoord.y) * vec2(g_constantVariables.renderTargetRes) - cameraPixelPos * 0.01;

    vec3 col = vec3(0.0);

    if (pixelCameraPos.y > (g_constantVariables.totalTime_HorizonPosition.y))
    {
        float pixelSkyPos = pixelCameraPos.y - g_constantVariables.totalTime_HorizonPosition.y;
        float normalizedPixelSkyPos = clamp(pixelSkyPos / 1280.0, 0.0, 1.0);
        
        float numberOfSteps = 5.0;
        float remainder = mod(normalizedPixelSkyPos * numberOfSteps, 1.0);
        int ditherPixel = 0;
        int dither = 0;

        if (remainder < 0.5)
        {
            ditherPixel = -1;
            float threshold = 0.0;
            int x = int(pixelCoord.x) % 4;
            int y = int(pixelCoord.y) % 4;
            threshold = bayerMatrix4x4[y][x];

            dither = int(remainder < (1.0 - threshold));
        }

        int quantizedSkyValue =  clamp(int(normalizedPixelSkyPos * numberOfSteps) + ditherPixel * dither, 0, int(numberOfSteps));
        col = skyColorTable[quantizedSkyValue];
    }
    else
    {
        float normalizedPixelGroundPos = clamp(pixelCameraPos.y / g_constantVariables.totalTime_HorizonPosition.y, 0.0, 1.0);
        
        float numberOfSteps = 1.0;
        float remainder = 1.0 - mod(normalizedPixelGroundPos * numberOfSteps, 1.0);
        bool dither = false;

        if (remainder < 0.5)
        {
            float threshold = 0.0;
            int x = int(pixelCoord.x) % 4;
            int y = int(pixelCoord.y) % 4;
            threshold = bayerMatrix4x4[y][x];

            dither = remainder < (1.0 - threshold);
        }
        
        col = dither ? vec3(205.0 / 255.0, 220.0 / 255.0, 222 / 255.0) : vec3(55.0 / 255.0, 47.0 / 255.0, 96.0 / 255.0);
    }

	outColor = vec4(col, 1.0);
}