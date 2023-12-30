#version 450

layout(location = 0) in vec4 vertexColor;
layout(location = 0) out uvec4 outColor;

void main() {
    outColor = uvec4(vertexColor * 255.0);
}