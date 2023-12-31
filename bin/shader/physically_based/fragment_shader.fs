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
    sampler2D ambient_maps[4];
    
    bool use_albedo_map;
    bool use_opacity_map;
    bool use_roughness_map;
    bool use_normal_map;
    bool use_displacement_map;
    bool use_metalic_map;
    bool use_ambient_map;

    vec3 albedo_color;
    float opacity;
    float roughness;
    float metalic;
    float normal_map_strength;
    vec3 ambient_color;
};

struct Point_light
{
    vec3 position;
    vec3 color;
    float diffuse;
    float specular;
    float ambient;
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
    vec3 direction;
    vec3 color;
    float diffuse;
    float specular;
    float ambient;
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
    float ambient;
    float intensity;
};

struct Skybox
{
    samplerCube irradiance_cubemap;
    samplerCube prefiltered_cubemap;
    sampler2D BRDF_LUT;
    bool use;
    float intensity;
    bool affect_scene;
};

layout(std140, Binding = 1) uniform FN
{
    float far;
    float near;
};

layout(std140, Binding = 2) uniform Light_matrices
{
    mat4 light_space_mat[8][10];  // light space matrix for directional lights
};

uniform Point_light point_lights[16];
uniform Spot_light spot_lights[16];
uniform Direction_light direction_lights[8];
uniform int point_lights_count;
uniform int spot_lights_count;
uniform int direction_lights_count;

uniform int cascade_count;
uniform float cascade_levels[10];
uniform vec3 view_pos;
uniform float bias;

uniform Material material;
uniform Skybox skybox;

uniform bool bloom;
uniform float threshold;

const float parallax_map_scale = .028f;
const float min_layer = 8.f;
const float max_layer = mix(32.f, 64.f, parallax_map_scale / 0.05f);
const float PI = 3.14159265359;

vec3 fresnelSchlickRoughness(vec3 F0, float cos_theta, float roughness);
vec3 fresnelSchlick(vec3 F0, float cos_theta);
float DistributionGGX(vec3 n, vec3 h, float roughness);
float GeometrySchlickGGX(vec3 n, vec3 v, float roughness);
float GeometrySmith(vec3 n , vec3 v, vec3 l, float roughness);
vec3 gamma_correct(vec3 color);
float linearize_depth(float F_depth, float near, float far);
float get_depth_view(float F_depth, float near, float far);
vec2 get_parallax_mapping_texcoord(Material material, vec2 texcoord, vec3 view_dir);
vec3 get_shading_normal(Material material, vec2 texture_coord);

vec3 cac_point_light(Point_light light, Material material);
vec3 cac_spot_light(Spot_light light, Material material);
vec3 cac_direction_light(Direction_light light, Material material, int index);
vec3 cac_env_irradiance(Skybox skybox, Material material);

float cac_point_shadow(Point_light light, vec3 light_dir);
float cac_spot_shadow(Spot_light light, vec3 light_dir);
float cac_direction_shadow(Direction_light light, int index);

void main()
{
    if((!material.use_opacity_map && material.opacity < 0.0001f) 
        || (material.use_opacity_map && texture(material.opacity_maps[0], fs_in.texture_coord).r < 0.0001f)) discard;

    vec3 result = vec3(0.f);
    for(int i = 0; i < point_lights_count; ++i)
        result += cac_point_light(point_lights[i], material);
    for(int i = 0; i < direction_lights_count; ++i)
        result += cac_direction_light(direction_lights[i], material, i);
    for(int i = 0; i < spot_lights_count; ++i)
        result += cac_spot_light(spot_lights[i], material);
    result += cac_env_irradiance(skybox, material);

    result = gamma_correct(result);
    float alpha = material.use_opacity_map ? texture(material.opacity_maps[0], fs_in.texture_coord).r : material.opacity;
    FragColor = vec4(result, alpha);
    if(bloom)
    {
        float brightness = dot(result, vec3(0.2126f, 0.7152f, 0.0722f));
        if(brightness > threshold) BrightColor = vec4(result, 1.f);
        else BrightColor = vec4(0.f, 0.f, 0.f, 1.f);
    }
    else BrightColor = vec4(0.f, 0.f, 0.f, 1.f);
}

vec3 fresnelSchlickRoughness(vec3 F0, float cos_theta, float roughness)
{
    return F0 + (max(vec3(1.f - roughness), F0) - F0) * pow(1.f - cos_theta, 5);
}

vec3 fresnelSchlick(vec3 F0, float cos_theta)
{
    return F0 + (vec3(1.f) - F0) * pow(1.f - cos_theta, 5);
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

float GeometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
    return GeometrySchlickGGX(n, v, roughness) * GeometrySchlickGGX(n, l, roughness);
}

vec3 gamma_correct(vec3 color)
{
    return pow(color, vec3(1.f / 2.2f));
}

vec3 cac_point_light(Point_light light, Material material)
{
    vec3 light_dir = normalize(light.position - vec3(fs_in.frag_pos));
    vec3 view_dir = normalize(view_pos - vec3(fs_in.frag_pos));
    vec2 texcoord = get_parallax_mapping_texcoord(material, fs_in.texture_coord, view_dir);
    vec3 normal = get_shading_normal(material, texcoord);
    vec3 h = normalize(view_dir + light_dir);
    float roughness = material.use_roughness_map ? texture(material.roughness_maps[0], texcoord).r : material.roughness;
    if(dot(view_dir, normalize(fs_in.frag_normal)) < 0 || dot(light_dir, normalize(fs_in.frag_normal)) < 0) return vec3(0.f);

    float n_dot_l = max(dot(normal, light_dir), 0);  // cosTheta

    float r = length(light.position - vec3(fs_in.frag_pos));
    float F_att = 1.f / (light.kc + light.kl * r + light.kq * r * r);
    vec3 radiance = light.color * F_att * light.intensity;  // Li(p, wi)

    vec3 albedo = material.use_albedo_map ? texture(material.albedo_maps[0], texcoord).rgb : material.albedo_color;
    float metalic = material.use_metalic_map ? texture(material.metalic_maps[0], texcoord).r : material.metalic;
    vec3 F0 = mix(vec3(0.04f), albedo, metalic);  // surface reflection at zero incidence
    float n_dot_v = max(dot(normal, view_dir), 0.f);
    vec3 F = fresnelSchlick(F0, max(dot(normalize(fs_in.frag_normal), view_dir), 0.f));  // F
    
    float NDF = DistributionGGX(normal, h, roughness);  // N

    float G = GeometrySmith(normal, view_dir, light_dir, roughness); // G
    
    vec3 kd = (vec3(1.f) - F) * (1 - metalic);
    vec3 L_oi = (kd * albedo / PI + F * NDF * G / max(4 * n_dot_l * n_dot_v, 0.0001f)) * radiance * n_dot_l;
    float in_shadow = cac_point_shadow(light, light_dir);
    return L_oi * (1.f - in_shadow);
}

vec3 cac_spot_light(Spot_light light, Material material)
{
    vec3 light_dir = normalize(light.position - vec3(fs_in.frag_pos));
    vec3 view_dir = normalize(view_pos - vec3(fs_in.frag_pos));
    vec2 texcoord = get_parallax_mapping_texcoord(material, fs_in.texture_coord, view_dir);
    vec3 normal = get_shading_normal(material, texcoord);
    vec3 h = normalize(view_dir + light_dir);
    float roughness = material.use_roughness_map ? texture(material.roughness_maps[0], texcoord).r : material.roughness;
    if(dot(view_dir, normalize(fs_in.frag_normal)) < 0 || dot(light_dir, normalize(fs_in.frag_normal)) < 0) return vec3(0.f);
    
    float n_dot_l = max(dot(normal, light_dir), 0);  // cosTheta
    
    float r = length(light.position - vec3(fs_in.frag_pos));
    float F_att = 1.f / (light.kc + light.kl * r + light.kq * r * r);
    vec3 radiance = light.color * F_att * light.intensity;  // Li(p, wi)

    vec3 albedo = material.use_albedo_map ? texture(material.albedo_maps[0], texcoord).rgb : material.albedo_color;
    float metalic = material.use_metalic_map ? texture(material.metalic_maps[0], texcoord).r : material.metalic;
    vec3 F0 = mix(vec3(0.04f), albedo, metalic);  // surface reflection at zero incidence
    float n_dot_v = max(dot(normal, view_dir), 0.f);
    vec3 F = fresnelSchlick(F0, max(dot(normalize(fs_in.frag_normal), view_dir), 0.f));  // F
    
    float NDF = DistributionGGX(normal, h, roughness);  // N

    float G = GeometrySmith(normal, view_dir, light_dir, roughness); // G
    
    vec3 kd = (vec3(1.f) - F) * (1 - metalic);
    vec3 L_oi = (kd * albedo / PI + F * NDF * G / max(4 * n_dot_l * n_dot_v, 0.0001f)) * radiance * n_dot_l;
    float in_shadow = cac_spot_shadow(light, light_dir);
    
    float theta = dot(-light_dir, light.direction), epslion = light.cutoff - light.outer_cutoff;
    float opacity = clamp((theta - light.outer_cutoff) / epslion, 0.f, 1.f);
    return L_oi * (1.f - in_shadow) * opacity;
}

vec3 cac_direction_light(Direction_light light, Material material, int index)
{
    vec3 light_dir = normalize(vec3(-light.direction));
    vec3 view_dir = normalize(view_pos - vec3(fs_in.frag_pos));
    vec2 texcoord = get_parallax_mapping_texcoord(material, fs_in.texture_coord, view_dir);
    vec3 normal = get_shading_normal(material, texcoord);
    vec3 h = normalize(view_dir + light_dir);
    float roughness = material.use_roughness_map ? texture(material.roughness_maps[0], texcoord).r : material.roughness;
    if(dot(view_dir, normalize(fs_in.frag_normal)) < 0 || dot(light_dir, normalize(fs_in.frag_normal)) < 0) return vec3(0.f);

    float n_dot_l = max(dot(normal, light_dir), 0);  // cosTheta

    vec3 radiance = light.color * light.intensity;  // Li(p, wi)

    vec3 albedo = material.use_albedo_map ? texture(material.albedo_maps[0], texcoord).rgb : material.albedo_color;
    float metalic = material.use_metalic_map ? texture(material.metalic_maps[0], texcoord).r : material.metalic;
    vec3 F0 = mix(vec3(0.04f), albedo, metalic);  // surface reflection at zero incidence
    float n_dot_v = max(dot(normal, view_dir), 0.f);
    vec3 F = fresnelSchlick(F0, max(dot(normalize(fs_in.frag_normal), view_dir), 0.f));  // F
    
    float NDF = DistributionGGX(normal, h, roughness);  // N

    float G = GeometrySmith(normal, view_dir, light_dir, roughness); // G
    
    vec3 kd = (vec3(1.f) - F) * (1 - metalic);
    vec3 L_oi = (kd * albedo / PI + F * NDF * G / max(4 * n_dot_l * n_dot_v, 0.0001f)) * radiance * n_dot_l;
    float in_shadow = cac_direction_shadow(light, index);
    return L_oi * (1.f - in_shadow);
}

vec3 cac_env_irradiance(Skybox skybox, Material material)
{
    if(!skybox.use) return vec3(0.f);
    if(!skybox.affect_scene) return vec3(0.f);

    vec3 view_dir = normalize(view_pos - vec3(fs_in.frag_pos));
    vec2 tex_coord = get_parallax_mapping_texcoord(material, fs_in.texture_coord, view_dir);
    vec3 normal = get_shading_normal(material, tex_coord);
    vec3 reflect_dir = reflect(-view_dir, normal);
    vec3 ambient = material.use_ambient_map ? texture(material.ambient_maps[0], tex_coord).rgb : material.ambient_color;

    vec3 albedo = material.use_albedo_map ? texture(material.albedo_maps[0], tex_coord).rgb : material.albedo_color;
    float roughness = material.use_roughness_map ? texture(material.roughness_maps[0], tex_coord).r : material.roughness;
    float metalic = material.use_metalic_map ? texture(material.metalic_maps[0], tex_coord).r : material.metalic;

    vec3 F0 = mix(vec3(0.04f), albedo, metalic);
    vec3 F = fresnelSchlickRoughness(F0, max(dot(fs_in.frag_normal, view_dir), 0.f), roughness);
    vec3 kd = (1.f - F) * (1 - metalic);  // nullify diffuse part of metalic surface
    
    vec3 diffuse_irradiance = texture(skybox.irradiance_cubemap, normal).rgb * albedo * kd;
    
    const float max_mip_level = 7.0f;
    vec3 prefilterd_specular = textureLod(skybox.prefiltered_cubemap, reflect_dir, roughness * max_mip_level).rgb;
    vec2 scale_bias = texture(skybox.BRDF_LUT, vec2(max(dot(normal, view_dir), 0.f), roughness)).rg;
    vec3 specular_irradiance = prefilterd_specular * (F * scale_bias.x + scale_bias.y);

    vec3 irradiance = (diffuse_irradiance + specular_irradiance) * skybox.intensity * ambient;
    return irradiance;
}

vec3 get_shading_normal(Material material, vec2 texture_coord)
{
    vec3 normal = material.use_normal_map ? normalize(fs_in.TBN * (texture(material.normal_maps[0], texture_coord).rgb * 2 - 1)) : normalize(fs_in.frag_normal);
    if(material.use_normal_map)
        normal = normalize(mix(normalize(fs_in.frag_normal), normal, material.normal_map_strength));
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

vec3 offset_vecs[20] = vec3[] 
    (
        vec3(-1.f, -1.f, -1.f), vec3(-1.f, -1.f, 0.f), vec3(-1.f, -1.f, 1.f), vec3(-1.f, 0.f, -1.f), vec3(-1.f, 0.f, 1.f), vec3(-1.f, 1.f, -1.f), vec3(-1.f, 1.f, 0.f), vec3(-1.f, 1.f, 1.f),
        vec3(0.f, -1.f, -1.f), vec3(0.f, -1.f, 1.f), vec3(0.f, 1.f, -1.f), vec3(0.f, 1.f, 1.f),
        vec3(1.f, -1.f, -1.f), vec3(1.f, -1.f, 0.f), vec3(1.f, -1.f, -1.f), vec3(1.f, 0.f, -1.f), vec3(1.f, 0.f, 1.f), vec3(1.f, 1.f, -1.f), vec3(1.f, 1.f, 0.f), vec3(1.f, 1.f, 1.f)
    );

float cac_point_shadow(Point_light light, vec3 light_dir)
{
    float bias_ = max(bias * (1 - dot(light_dir, normalize(fs_in.frag_normal))), bias * 0.01f);
    float in_shadow = 0.f;

    float radius = (1.f + linearize_depth(gl_FragCoord.z, near, far)) / 75.f;
    vec3 light2frag = vec3(fs_in.frag_pos) - light.position;
    float depth_current = length(light2frag);

    float depth_max = length(light2frag) * min(light.far / abs(light2frag.x), min(light.far / abs(light2frag.y), light.far / abs(light2frag.z)));
    float depth_min = length(light2frag) * min(light.near / abs(light2frag.x), min(light.near / abs(light2frag.y), light.near / abs(light2frag.z)));
    for(int i = 0; i < 20; ++i)
    {
        float depth_closest = texture(light.depth_cubemap, light2frag + radius * offset_vecs[i]).r;  // [0, 1], z_screen
        depth_closest = depth_closest * (depth_max - depth_min) + depth_min;  // [depth_min, depth_max]
        in_shadow += ((depth_closest + bias_) < depth_current) ? 1.f / 20.f : 0.f;
    }

    return in_shadow;
}

float cac_direction_shadow(Direction_light light, int index)
{
    float in_shadow = 0.f;
    float bias_ = max(bias * (1.f - dot(normalize(fs_in.frag_normal), normalize(-light.direction))), bias * 0.01f);
    float z_view = get_depth_view(gl_FragCoord.z, near, far);
    
    int shadow_index = cascade_count - 1;
    for(int i = 0; i < cascade_count - 1; ++i)
    {
        if(z_view < cascade_levels[i])
        {
            shadow_index = i;
            break;
        }
    }
    
    bias_ *= (shadow_index == cascade_count - 1) ? (0.1f / far) : (0.1f / cascade_levels[shadow_index]);
    vec4 light_space_clip = light_space_mat[index][shadow_index] * fs_in.frag_pos;
    vec3 light_space_ndc = light_space_clip.xyz / light_space_clip.w;
    vec3 light_space_screen = light_space_ndc * 0.5f + 0.5f;
    float depth_current = light_space_screen.z;  // depth value in light screen space, range: [0, 1]
    if(depth_current >= 1.f) return 0.f;
    vec2 texel_size = 1.f / textureSize(light.cascade_maps, 0).xy;

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float depth_closest = texture(light.cascade_maps, 
                vec3(light_space_screen.xy + vec2(x, y) * texel_size, shadow_index)).r;  // range: [0, 1]
            in_shadow += ((depth_closest) < depth_current) ? 1.f / 9.f : 0.f;
        }
    }
    return in_shadow;
}

float cac_spot_shadow(Spot_light light, vec3 light_dir)
{
    float bias_ = max(bias * (1 - dot(light_dir, normalize(fs_in.frag_normal))), bias * 0.01f);
    float in_shadow = 0.f;

    float radius = (1.f + linearize_depth(gl_FragCoord.z, near, far)) / 75.f;
    vec3 light2frag = vec3(fs_in.frag_pos) - light.position;
    float depth_current = length(light2frag);

    float depth_max = length(light2frag) * min(light.far / abs(light2frag.x), min(light.far / abs(light2frag.y), light.far / abs(light2frag.z)));
    float depth_min = length(light2frag) * min(light.near / abs(light2frag.x), min(light.near / abs(light2frag.y), light.near / abs(light2frag.z)));
    for(int i = 0; i < 20; ++i)
    {
        float depth_closest = texture(light.depth_cubemap, light2frag + radius * offset_vecs[i]).r;  // [0, 1], z_screen
        depth_closest = depth_closest * (depth_max - depth_min) + depth_min;  // [depth_min, depth_max]
        in_shadow += ((depth_closest + bias_) < depth_current) ? 1.f / 20.f : 0.f;
    }

    return in_shadow;
}

float linearize_depth(float F_depth, float near, float far)
{
    float z_view = (near * far) / (far - F_depth * (far - near));
    return (z_view - near) / (far - near);
}

float get_depth_view(float F_depth, float near, float far)
{
    return (near * far) / (far - F_depth * (far - near));
}
