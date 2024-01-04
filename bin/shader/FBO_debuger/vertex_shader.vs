# version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 tex_coord;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(aPos, 0.f, 1.f);
    tex_coord = aTexCoord;
}