# version 460 core

in vec2 tex_coord;

out vec4 FragColor;

uniform bool alpha;
uniform sampler2D fbo_attachment;

vec3 getSampleColorMS(sampler2DMS imageMS, ivec2 coord);

void main()
{
    FragColor = alpha ? vec4(vec3(texture(fbo_attachment, tex_coord).a), 1.f) : vec4(texture(fbo_attachment, tex_coord).rgb, 1.f);
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
