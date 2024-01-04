# version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexcoord;

out vec2 tex_coord;

void main()
{
    gl_Position = vec4(aPos, 0.f, 1.f);  // clip = vec4(aPos, 0.f, 1.f)->ndc = vec4(aPos, 0.f, 1.f)
    tex_coord = aTexcoord;
}