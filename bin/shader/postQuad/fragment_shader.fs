#version 460 core

in vec2 textureCoord;

uniform sampler2DMS sceneColorMS;
uniform sampler2DMS bluredBrightnessMS;
uniform sampler2D sceneColor;
uniform sampler2D bluredBrightness;
uniform bool hdr;
uniform bool bloom;
uniform float exposure;
uniform bool defered;

// default frambuffer
out vec4 FragColor;

vec4 getSceneColorMS(sampler2DMS sceneColorMS);
vec3 getBrightnessMS(sampler2DMS bluredBrightnessMS);

void main()
{
    vec4 result = vec4(0.f);
    result = defered ? texture(sceneColor, textureCoord) : getSceneColorMS(sceneColorMS);  // color
    // if(sample_alpha <= 0.0001f) discard;

    // Apply bloom effect
    if(bloom && result.a > 0.0001f) 
        result.rgb += (defered ? texture(bluredBrightness, textureCoord).rgb : getBrightnessMS(bluredBrightnessMS)) * 1.3f;

    // Apply tone mapping
    if(hdr && result.a > 0.0001f) result.rgb = vec3(1.f) - exp(-result.rgb * exposure);

    FragColor = vec4(result.rgb, 1.f);
}

vec4 getSceneColorMS(sampler2DMS sceneColorMS)
{
    vec3 sample_color = vec3(0.f);
    float sample_alpha = 0.f;
    // resolve ms texture to default texture
    for(int i = 0; i < 4; ++i)
    {
        vec4 sub_sample = texelFetch(sceneColorMS, ivec2(gl_FragCoord.xy), i);
        sample_color += sub_sample.rgb;
        sample_alpha = max(sample_alpha, sub_sample.a);
    }
    sample_color *= .25f;

    return vec4(sample_color, sample_alpha);
}

vec3 getBrightnessMS(sampler2DMS bluredBrightnessMS)
{
    vec3 brightnessSample = vec3(0.f);
    for(int i = 0; i < 4; ++i)
        brightnessSample += texelFetch(bluredBrightnessMS, ivec2(gl_FragCoord.xy), i).rgb;
    brightnessSample *= .25f;
    return brightnessSample;
}