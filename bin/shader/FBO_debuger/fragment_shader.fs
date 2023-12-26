# version 460 core

in vec2 tex_coord;

out vec4 FragColor;

uniform sampler2D fbo_attachment;

void main()
{
    FragColor = vec4(texture(fbo_attachment, tex_coord).rgb, 1.f);
}