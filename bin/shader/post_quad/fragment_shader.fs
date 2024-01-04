#version 460 core

in vec2 texture_coord;

uniform sampler2DMS image;
uniform sampler2DMS blured_brightness;
uniform bool hdr;
uniform bool bloom;
uniform float exposure;

out vec4 FragColor;


void main()
{
    vec3 sample_color = vec3(0.f);
    float sample_alpha = 0.f;
    for(int i = 0; i < 4; ++i)
    {
        vec4 sub_sample = texelFetch(image, ivec2(gl_FragCoord.xy), i);
        sample_color += sub_sample.rgb;
        sample_alpha = max(sample_alpha, sub_sample.a);
    }
    sample_color *= .25f;
    // if(sample_alpha <= 0.0001f) discard;

    vec3 result = sample_color;
    if(bloom && sample_alpha > 0.0001f)
    {
        vec3 brighness_sample = vec3(0.f);
        for(int i = 0; i < 4; ++i)
            brighness_sample += texelFetch(blured_brightness, ivec2(gl_FragCoord.xy), i).rgb;
        brighness_sample *= .25f;
        result += brighness_sample;
    }
    if(hdr && sample_alpha > 0.0001f) result = vec3(1.f) - exp(-result * exposure);
    FragColor = vec4(result, 1.f);
}