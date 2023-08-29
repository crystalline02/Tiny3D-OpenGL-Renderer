#version 460 core
out vec4 FragColor;

uniform vec3 single_color;

void main()
{
    float gamma = 2.2f;
    FragColor = vec4(pow(single_color, 1.f / vec3(gamma)), 1.f);
}