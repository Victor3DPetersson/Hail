#version 450
layout(location = 0) in uint inIndex;

layout( std140, set = 0, binding = 1) uniform PerCameraDataBuffer {
    mat4 cameraView;
    mat4 cameraProj;
} perCameraUbo;

struct DebugLineData
{
    vec4 posAndIs2D;
    vec4 color; 
};

layout(std140, set = 1, binding = 0) buffer readonly LineDataBuffer 
{
   	DebugLineData lineDataBuffer[];
};


layout(location = 0) out vec4 outColor;

void main() 
{
	DebugLineData lineData = lineDataBuffer[inIndex];

    vec4 finalPosition = vec4(0.0);
    // 3D debug line, w is 1.0 to be multiplieable with the matrixes
    if (lineData.posAndIs2D.w > 0.0)
    {
        finalPosition = perCameraUbo.cameraProj * perCameraUbo.cameraView * lineData.posAndIs2D;
    }
    else
    {
        finalPosition = vec4(lineData.posAndIs2D.xy, 0.0001, 1.0);
    }
	gl_Position = finalPosition;
	outColor = lineData.color;
}