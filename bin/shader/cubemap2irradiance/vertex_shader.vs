# version 460 core

layout(location = 0) in vec3 aPos;

out vec3 frag_pos;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    frag_pos = aPos;  // world
    gl_Position = projection * view * vec4(aPos, 1.f);  // wolrd->clip
}