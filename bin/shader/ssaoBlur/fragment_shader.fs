#version 460 core
out float FragColor;

in vec2 texture_coord;

uniform sampler2D ssao_buffer;

void main()
{
    // Mean filter to bulr ssao_buffer
    float result = 0.f;
    vec2 texel_size = 1.f / textureSize(ssao_buffer, 0);
    for(int x = -2; x < 2; ++x)
    {
        for(int y = -2; y < 2; ++y)
        {
            result += texture(ssao_buffer, texture_coord + vec2(x * texel_size.x, y * texel_size.y)).r;
        }
    }
    result /= 16.f;
    FragColor = result;
}