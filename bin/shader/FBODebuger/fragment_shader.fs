# version 460 core

in vec2 tex_coord;

out vec4 FragColor;

uniform bool alpha;
uniform sampler2D fbo_attachment;

void main()
{
    FragColor = alpha ? vec4(vec3(texture(fbo_attachment, tex_coord).a), 1.f) : vec4(vec3(texture(fbo_attachment, tex_coord).r), 1.f);
}