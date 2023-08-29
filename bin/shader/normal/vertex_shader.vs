#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat3 normal_mat;

out vec3 normal;

void main()
{
    gl_Position = model * vec4(aPos, 1.f);  // local space->world space
    normal = normalize(normal_mat * aNormal);  // local space->world space
}