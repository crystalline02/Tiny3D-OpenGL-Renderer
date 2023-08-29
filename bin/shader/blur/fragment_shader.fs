#version 460 core

in vec2 texture_coord;
out vec4 FragColor;

uniform sampler2DMS image;
uniform bool horizental;

float weight[5] = float[] (0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);

void main()
{
    vec3 result = vec3(0.f);
    vec3 image_sample = vec3(0.f);
    for(int i = 0; i < 4; ++i)
    {
        vec4 sub_sample = texelFetch(image, ivec2(gl_FragCoord.xy), i);
        image_sample += sub_sample.a == 0.f ? vec3(0.f) : sub_sample.rgb;
    }
    result += image_sample / 4.f * weight[0];
    for(int i = 1; i < 5; ++i)
    {
        image_sample = vec3(0.f);
        for(int j = 0; j < 4; ++j)
        {
            vec4 sub_sample = texelFetch(image, ivec2(gl_FragCoord.xy) + (horizental ? ivec2(i, 0) : ivec2(0, i)), j);
            image_sample += sub_sample.a == 0.f ? vec3(0.f) : sub_sample.rgb;
        }
        result += image_sample / 4.f * weight[i];

        image_sample = vec3(0.f);
        for(int j = 0; j < 4; ++j)
        {
            vec4 sub_sample = texelFetch(image, ivec2(gl_FragCoord.xy) - (horizental ? ivec2(i, 0) : ivec2(0, i)), j);
            image_sample += sub_sample.a == 0.f ? vec3(0.f) : sub_sample.rgb;
        }
        result += image_sample / 4.f * weight[i];
    }
    FragColor = vec4(result, 1.f);
}