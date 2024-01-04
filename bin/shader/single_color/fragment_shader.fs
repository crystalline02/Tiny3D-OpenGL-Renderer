#version 460 core
out vec4 FragColor;

uniform vec3 single_color;

void main()
{
    FragColor = vec4(single_color, 1.f);
}