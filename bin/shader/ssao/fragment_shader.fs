#version 460 core

in vec2 texture_coord;
// SSAOFBO
out float FragColor;

layout(std140) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

uniform sampler2D position_buffer;
uniform sampler2D surface_normal;
uniform sampler2D noise_tex;
uniform float radius;
uniform vec3 samples[64];

const float bias = 0.0001f;
const float kernel_size = 64.f;

void main()
{
    vec3 frag_normal = texture(surface_normal, texture_coord).rgb;  // world
    vec3 frag_pos = texture(position_buffer, texture_coord).rgb;  // world
    vec3 random_vector = texture(noise_tex, texture_coord.xy * vec2(1920.f / 4.f, 1080.f / 4.f)).rgb;
    vec3 tangent = normalize(random_vector - dot(random_vector, frag_normal) * frag_normal);  // wolrd
    vec3 bitangent = normalize(cross(frag_normal, tangent));
    mat3 TBN = mat3(tangent, bitangent, frag_normal);

    float occulsion = 0.f;
    for(int i = 0; i < kernel_size; ++i)
    {
        vec3 sample_tangent = samples[i];
        vec3 offset_world = TBN * sample_tangent;
        vec3 sample_world = offset_world * radius + frag_pos;
        vec4 sample_view = view * vec4(sample_world, 1.f);
        vec4 sample_clip = projection * sample_view;
        vec3 sample_ndc = sample_clip.xyz / sample_clip.w;
        vec3 sample_screen_coord = sample_ndc * 0.5f + 0.5f;
        float z_fragment = (view * vec4(texture(position_buffer, sample_screen_coord.xy).rgb, 1.f)).z;

        float range_check = smoothstep(0.f, 1.f, radius / abs(z_fragment - sample_view.z));
        occulsion += (z_fragment > sample_view.z + bias ? 1.f : 0.f) * range_check;
    }
    occulsion = 1.f - occulsion / kernel_size;

    FragColor = occulsion;
}