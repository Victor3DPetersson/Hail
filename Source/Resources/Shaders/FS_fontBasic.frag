#version 450
#extension GL_EXT_fragment_shader_barycentric : require
#extension GL_EXT_mesh_shader : require

layout(location = 0) out vec4 outColor;

layout (location = 0) in flat uint triangleType;
layout (location = 1) in flat uint triangleID;

const uint SOLID = 2;
const uint CONVEX = 1;
const uint CONCAVE = 0;

vec2 computeUV(vec3 bary)
{
    float u = bary.x * 0.0 + bary.y * 0.5 + bary.z * 1.0;
    float v = bary.x * 0.0 + bary.y * 0.0 + bary.z * 1.0;
    return vec2(u, v);
}

float computeQuadraticBezierFunction(const vec2 uv)
{
    return uv.x * uv.x - uv.y;
}

void main()
{
    const vec2 uv = computeUV(gl_BaryCoordEXT);
    const float y = computeQuadraticBezierFunction(uv);
    float triangleType = float(triangleType);
    float triangleID = float(triangleID);

    vec3 color = vec3(0.95, 0.95, 0.95);
    //color = vec3(uv, 0.7);
    if (((triangleType == CONVEX) && (y > 0.0)) || ((triangleType == CONCAVE) && (y < 0.0)))
    {
        discard;                        
    }
    outColor = vec4(color, 1);
}