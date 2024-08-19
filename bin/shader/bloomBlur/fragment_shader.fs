#version 460 core

in vec2 texture_coord;
out vec4 FragColor;

uniform sampler2DMS imageMS;
uniform sampler2D image;
uniform bool horizental;
uniform bool defered;

float weight[9] = float[] (0.13357122, 0.12635296, 0.10695547, 0.08101504, 0.05491277, 0.03330628, 0.0180769, 0.00877944, 0.00381553);

vec3 getSampleColorMS(sampler2DMS imageMS, ivec2 coord);
vec3 getSampleColor(sampler2D image, ivec2 coord);

void main()
{
    ivec2 screenCoord = ivec2(gl_FragCoord.xy);
    vec3 result = vec3(0.f);
    result += (defered ? getSampleColor(image, screenCoord) : getSampleColorMS(imageMS, screenCoord)) * weight[0];
    for(int i = 1; i < 9; ++i)
    {
        result += (defered ? getSampleColor(image, screenCoord + (horizental ? ivec2(i, 0) : ivec2(0, i))) : getSampleColorMS(imageMS, screenCoord + (horizental ? ivec2(i, 0) : ivec2(0, i)))) * weight[i];
        result += (defered ? getSampleColor(image, screenCoord - (horizental ? ivec2(i, 0) : ivec2(0, i))) : getSampleColorMS(imageMS, screenCoord - (horizental ? ivec2(i, 0) : ivec2(0, i)))) * weight[i];
    }

    FragColor = vec4(result, 1.f);
}

vec3 getSampleColorMS(sampler2DMS imageMS, ivec2 coord)
{
    vec3 result = vec3(0.f);
    for(int i = 0; i < 4; ++i)
    {
        vec4 sub_sample = texelFetch(imageMS, coord, i);
        result += sub_sample.a <= 1e-4f ? vec3(0.f) : sub_sample.rgb;
    }
    return result / 4.f;
}

vec3 getSampleColor(sampler2D image, ivec2 coord)
{
    vec4 sampleRes = texelFetch(image, coord, 0);
    // [重度修复] 来自sceneColorAttachments[1]的GL_TEXTURE_2D，表示的是按照阈值提取出的高亮像素，由于defered中GBuffer数值精度的问题，有可能计算出负数值！
    return sampleRes.a <= 1e-4f ? vec3(0.f) : max(sampleRes.rgb, vec3(0.f));
}