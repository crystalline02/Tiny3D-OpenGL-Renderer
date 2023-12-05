#version 460 core

in GS_OUT
{
    vec3 frag_normal;  // world
    vec4 frag_pos;  // world
    vec2 texture_coord; // UV
    mat3 TBN;  // world
} fs_in;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

struct Material
{
    sampler2D albedo_maps[4];
    sampler2D opacity_maps[4];
    sampler2D roughness_maps[4];
    sampler2D normal_maps[4];
    sampler2D displacement_maps[4];
    sampler2D metalic_maps[4];
    
    bool use_albedo_map;
    bool use_opacity_map;
    bool use_roughness_map;
    bool use_normal_map;
    bool use_displacement_map;
    bool use_metalic_map;

    vec3 albedo_color;
    float opacity;
    float roughness;
    float metalic;
    float normal_map_strength;
};

struct Point_light
{
    vec3 position;
    vec3 color;
    float diffuse;
    float specular;
    float intensity;
    float kc;
    float kl;
    float kq;
    float near;
    float far;
    samplerCube depth_cubemap;
};

struct Spot_light
{
    vec3 position;
    vec3 color;
    float diffuse;
    float specular;
    float intensity;
    float outer_cutoff;
    float cutoff;
    float kc;
    float kl;
    float kq;
    float near;
    float far;
    samplerCube depth_cubemap;
};

struct Direction_light
{
    vec3 direction;
    sampler2DArray cascade_maps;
    vec3 color;
    float diffuse;
    float specular;
    float intensity;
};

struct Skybox
{
    samplerCube cubemap;
    bool use;
    float intensity;
};

layout(std140, Binding = 1) uniform FN
{
    float far;
    float near;
};

layout(std140, Binding = 2) uniform Light_matrices
{
    mat4 light_space_mat[8][10];
};

uniform Point_light point_lights[16];
uniform Spot_light spot_lights[16];
uniform Direction_light direction_lights[8];
uniform int point_lights_count;
uniform int spot_lights_count;
uniform int direction_lights_count;

uniform vec3 view_pos;

uniform Material material;
uniform Skybox skybox;

const float parallax_map_scale = .028f;
const float min_layer = 8.f;
const float max_layer = mix(32.f, 64.f, parallax_map_scale / 0.05f);
const float PI = 3.14159265359;

vec3 fresnelSchlick(vec3 F0, float cos_theta);
float DistributionGGX(vec3 n, vec3 h, float roughness);
float GeometrySchlickGGX(vec3 n, vec3 v, float roughness);
float GeometrySmith(vec3 n , vec3 v, vec3 l, float roughness);
vec2 get_parallax_mapping_texcoord(Material material, vec2 texcoord, vec3 view_dir);
vec3 get_shading_normal(Material material, vec2 texture_coord);
vec3 cac_point_light(Point_light light, Material material);

void main()
{
    if((!material.use_opacity_map && material.opacity < 0.0001f) 
        || (material.use_opacity_map && texture(material.opacity_maps[0], fs_in.texture_coord).r < 0.0001f)) discard;
    vec3 result = vec3(0.f);
    for(int i = 0; i < point_lights_count; ++i)
    {
        
    }
}

vec3 fresnelSchlick(vec3 F0, float cos_theta)
{
    return F0 + (1 - F0) * pow(1. - cos_theta, 5);
}

float DistributionGGX(vec3 n, vec3 h, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float n_dot_h = max(dot(n, h), 0.f);
    float n_dot_h2 = n_dot_h * n_dot_h;
    float denominator = n_dot_h2 * (a2 - 1) + 1;
    denominator = denominator * denominator * PI;
    return a2 / denominator;
}

float GeometrySchlickGGX(vec3 n, vec3 v, float roughness)
{
    float k = (roughness + 1) * (roughness + 1) / 8;
    float n_dot_v = max(dot(n, v), 0);
    float numerator = n_dot_v;
    float denominator = n_dot_v * (1 - k) + k;
    return numerator / denominator;
}

float GeometrySmith(vec3 n , vec3 v, vec3 l, float roughness)
{
    return GeometrySchlickGGX(n, v, roughness) * GeometrySchlickGGX(n, l, roughness);
}

vec3 cac_point_light(Point_light light, Material material)
{
    vec3 light_dir = normalize(light.position - vec3(fs_in.frag_pos));
    vec3 view_dir = normalize(view_pos - vec3(fs_in.frag_pos));
    float r = length(light.position - vec3(fs_in.frag_pos));
    float F_att = light.kc + light.kl * r + light.kq * r * r;
    vec3 radiance = light.color * F_att;
    vec3 normal = get_shading_normal(material, fs_in.texture_coord);

    vec3 albedo_color = material.use_albedo_map ? texture(material.albedo_maps[0], fs_in.texture_coord).rgb : material.albedo_color;
    float metalic = material.use_metalic_map ? texture(material.metalic_maps[0], fs_in.texture_coord).r : material.metalic;
    vec3 F0 = mix(vec3(0.04f), albedo_color, metalic);  // surface reflection at zero incidence
    vec3 F = fresnelSchlick(F0, max(dot(view_dir, normal), 0.f));
    return vec3(0.f);
}

vec3 get_shading_normal(Material material, vec2 texture_coord)
{
    vec3 normal = material.use_normal_map ? normalize(fs_in.TBN * (texture(material.normal_maps[0], texture_coord).rgb * 2 - 1)) : normalize(fs_in.frag_normal);
    if(material.use_normal_map)
        normal = normalize(mix(fs_in.frag_normal, normal, material.normal_map_strength));
    return normal;
}

vec2 get_parallax_mapping_texcoord(Material material, vec2 texcoord, vec3 view_dir)
{
    if(!material.use_displacement_map || parallax_map_scale == 0.f) return texcoord;
    vec3 view_dir_tangent = normalize(transpose(fs_in.TBN) * view_dir);
    float num_layers = mix(max_layer, min_layer, max(dot(vec3(0.f, 0.f, 1.f), view_dir_tangent), 0));
    float layer_depth = parallax_map_scale / num_layers;
    vec2 delta_texcoords = view_dir_tangent.xy * parallax_map_scale / (view_dir_tangent.z * num_layers);

    float current_depth_value = texture(material.displacement_maps[0], texcoord).r * parallax_map_scale;
    vec2 current_texcoord = texcoord;
    float current_layer_depth = 0.f;
    while(current_layer_depth < current_depth_value)
    {
        current_texcoord -= delta_texcoords;
        current_layer_depth += layer_depth;
        current_depth_value = texture(material.displacement_maps[0], current_texcoord).r * parallax_map_scale;
    }

    vec2 prev_texcoord = current_texcoord + delta_texcoords;
    float depth_after = abs(current_layer_depth - current_depth_value);
    float depth_before = abs(texture(material.displacement_maps[0], prev_texcoord).r * parallax_map_scale - current_layer_depth + layer_depth);
    float weight = depth_after / (depth_after + depth_before);
    return mix(current_texcoord, prev_texcoord, weight);
}