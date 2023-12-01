#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTextureCoord;

out VS_OUT
{
    vec3 normal;  // world
    vec3 tangent;  // world
    vec2 texture_coord;  // UV
} vs_out;

uniform mat4 model;
uniform mat3 normal_mat;

void main()
{
    gl_Position = model * vec4(aPos, 1.f);  // world
    vs_out.normal = normalize(normal_mat * aNormal);  // world
    vs_out.tangent = normalize(normal_mat * aTangent);  // world
    vs_out.texture_coord = aTextureCoord;
}