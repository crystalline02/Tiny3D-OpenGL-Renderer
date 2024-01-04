# version 460 core
layout(location = 0) in vec4 pos_tex;

out vec2 tex_coord;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(pos_tex.xy, 0.f, 1.f);
    tex_coord = pos_tex.zw;
}