#version 450

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1, set = 1) uniform sampler2D texSampler;

layout(location = 0) out uvec4 outColor;

void main() {
    vec4 color = texture(texSampler, vec2(1.0-fragTexCoord.x, fragTexCoord.y)).rgba;
    outColor = uvec4(color * 255.0);
}