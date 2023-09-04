#version 460 core

in vec2 texture_coord;
out vec4 FragColor;

uniform sampler2DMS image;
uniform bool horizental;

float weight[9] = float[] (0.13357122, 0.12635296, 0.10695547, 0.08101504, 0.05491277, 0.03330628, 0.0180769, 0.00877944, 0.00381553);

vec3 get_sample_ms(sampler2DMS image, ivec2 coord);

void main()
{
    vec3 result = vec3(0.f);
    vec3 image_sample = vec3(0.f);
    result += get_sample_ms(image, ivec2(gl_FragCoord.xy)) * weight[0];
    for(int i = 1; i < 9; ++i)
    {
        result += get_sample_ms(image, ivec2(gl_FragCoord.xy + (horizental ? ivec2(i, 0) : ivec2(0, i)))) * weight[i];
        result += get_sample_ms(image, ivec2(gl_FragCoord.xy - (horizental ? ivec2(i, 0) : ivec2(0, i)))) * weight[i];
    }
    FragColor = vec4(result, 1.f);
}

vec3 get_sample_ms(sampler2DMS image, ivec2 coord)
{
    vec3 result = vec3(0.f);
    for(int i = 0; i < 4; ++i)
    {
        vec4 sub_sample = texelFetch(image, coord, i);
        result += sub_sample.a <= 0.0001f ? vec3(0.f) : sub_sample.rgb;
    }
    return result / 4.f;
}