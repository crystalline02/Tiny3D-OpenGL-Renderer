#version 460 core
layout(location = 0) in vec3 aPos;

layout(std140, Binding = 0) uniform Matrices
{
	mat4 view;  // base alignment: 16 * 4 = 64, alignment offset: 0 
	mat4 projection;  // base alignment: 16 * 4 = 64, alignment offset: 64
};

uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.f);  // world space
}