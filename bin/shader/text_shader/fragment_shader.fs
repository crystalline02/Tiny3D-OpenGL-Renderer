# version 460 core

in vec2 tex_coord;

out vec4 FragColor;

uniform sampler2D bitmap;
uniform vec3 color;

void main()
{
    float sampled = texture(bitmap, vec2(tex_coord.x, 1.f - tex_coord.y)).r;
    FragColor = vec4(color, 1.f) * sampled;
}