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

void main()
{
    
}