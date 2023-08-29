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

void main()
{
    vec4 image_sample = vec4(0.f);
    for(int i = 0; i < 4; ++i)
    {
        vec4 sub_sample = texelFetch(image, ivec2(gl_FragCoord.xy), i);
        image_sample += sub_sample.a == 0.f ? vec4(0.f) : sub_sample;
    }
    image_sample /= 4.f;
    if(image_sample.a == 0.f) discard;

    vec3 color = image_sample.rgb;
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
    if(gamma_correct) color = pow(color, vec3(1.f / gamma));
    FragColor = vec4(color, 1.f);
}