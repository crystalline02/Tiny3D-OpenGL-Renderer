#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;

uniform mat4 model;
uniform mat3 normal_mat;

out VS_OUT 
{
    vec3 normal;  // world space
    vec3 tangent;  // world space
} vs_out;

void main()
{
    gl_Position = model * vec4(aPosition, 1.f);  // loacal space->world space
    vs_out.normal = normalize(normal_mat * aNormal);  // local space->world space
    vs_out.tangent = normalize(normal_mat * aTangent);  // local space->world space
}