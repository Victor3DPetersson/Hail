#version 450

// TODO: create a utility header for shaders with common operations
vec3 ColorFromPackedColor(uint packedColor)
{
    vec3 returnColor;
    returnColor.x = float((packedColor >> 24) & 0xff) / 255.0;
    returnColor.y = float((packedColor >> 16) & 0xff) / 255.0;
    returnColor.z = float((packedColor >> 8) & 0xff) / 255.0;
    return returnColor;
}

layout(location = 0) in vec2 TexCoord;
layout(location = 1) in flat uint vertexColor;
layout(location = 0) out vec4 outColor;

void main() {

    float distanceToCenterSquared = length(TexCoord); 
    vec3 color = ColorFromPackedColor(vertexColor);
    
    if (distanceToCenterSquared > 1.0)
        discard;

    outColor = vec4(color, 1.0);
}