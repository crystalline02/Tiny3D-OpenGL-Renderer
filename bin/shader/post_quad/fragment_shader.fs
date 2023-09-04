#version 460 core

in vec2 texture_coord;

uniform sampler2DMS image;
uniform sampler2DMS blured_brightness;
uniform bool hdr;
uniform bool gamma_correct;
uniform bool bloom;
uniform float exposure;

out vec4 FragColor;

const float gamma = 2.2f;

vec3 gamma_correction(vec3 color);
vec4 gamma_decorrection(vec4 color);

void main()
{
    vec4 image_sample = vec4(0.f);
    for(int i = 0; i < 4; ++i)
    {
        vec4 sub_sample = texelFetch(image, ivec2(gl_FragCoord.xy), i);
        image_sample += sub_sample.a <= 0.0001f ? gamma_decorrection(sub_sample) : sub_sample;
    }
    image_sample /= 4.f;
    if(image_sample.a <= 0.0001f) discard;

    vec3 color = image_sample.rgb;
    vec3 brightness_image = vec3(0.f);
    if(bloom)
    {
        vec3 brighness_sample = vec3(0.f);
        for(int i = 0; i < 4; ++i)
            brighness_sample += texelFetch(blured_brightness, ivec2(gl_FragCoord.xy), i).rgb;
        brighness_sample /= 4.f;
        // FragColor = vec4(brighness_sample, 1.f);
        color += brighness_sample;
    }
    if(hdr) color = vec3(1.f) - exp(-color * exposure);
    if(gamma_correct) color = gamma_correction(color);
    FragColor = vec4(color, 1.f);
}

vec3 gamma_correction(vec3 color)
{
    return pow(color, vec3(1.f / gamma));
}

vec4 gamma_decorrection(vec4 color)
{
    return vec4(pow(color.rgb, vec3(gamma)), color.a);
}
