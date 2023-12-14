# version 460 core

layout(location = 0) in vec3 aPos;  // world

out vec3 frag_pos;  // world

uniform mat4 projection;
uniform mat4 view;

void main()
{
    frag_pos = aPos;
    gl_Position = projection * view * vec4(aPos, 1.f);  // world->view->clip
}