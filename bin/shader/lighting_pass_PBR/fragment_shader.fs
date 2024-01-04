#version 460 core
in vec2 texture_coord;
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

struct Point_light
{
    vec3 position;  // sub common
    vec3 color;  // common
    float diffuse;  // common
    float specular;  // common
    float ambient;  // common
    float intensity;  // common
    float kc;  // sub common
    float kl;  // sub common
    float kq;  // sub common
    float near;  // sub common
    float far;  // sub common
    samplerCube depth_cubemap;  // sub common
};

struct Spot_light
{
    vec3 position;  // sub common
    vec3 direction;  // spot
    vec3 color;  // common
    float diffuse;  // common
    float specular;  // common
    float ambient;  // common
    float intensity;  // common
    float outer_cutoff;  // spot
    float cutoff;  // spot
    float kc;  // sub common
    float kl;  // sub common
    float kq;  // sub common
    float near;  // sub common
    float far;  // sub common
    samplerCube depth_cubemap;  // sub common
};

struct Direction_light
{
    vec3 direction;  // dir
    sampler2DArray cascade_maps;  // dir
    vec3 color;  // common
    float diffuse;  // common
    float specular;  // common
    float ambient;  // common
    float intensity;  // commons
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
    mat4 light_space_mat[8][10];
};

uniform sampler2D position_buffer;
uniform sampler2D normal_depth;
uniform sampler2D surface_normal;
uniform sampler2D albedo_specular;
uniform sampler2D ambient_metalic;
uniform sampler2D ssao_buffer;
uniform int point_lights_count;
uniform int spot_lights_count;
uniform int direction_lights_count;
uniform Point_light point_lights[16];
uniform Spot_light spot_lights[16];
uniform Direction_light direction_lights[16];
uniform Skybox skybox;
uniform vec3 view_pos;
uniform float bias;
uniform float cascade_levels[10];
uniform int cascade_count;
uniform float threshold;
uniform bool ssao;

vec3 cac_point_light(Point_light light);
vec3 cac_spot_light(Spot_light light);
vec3 cac_direction_light(Direction_light light, int index);
vec3 cac_env_irradiance(Skybox skybox);

vec3 fresnelSchlickRoughness(vec3 F0, float cos_theta, float roughness);
vec3 fresnelSchlick(vec3 F0, float cos_theta);
float DistributionGGX(vec3 n, vec3 h, float roughness);
float GeometrySchlickGGX(vec3 n, vec3 v, float roughness);
float GeometrySmith(vec3 n , vec3 v, vec3 l, float roughness);

float cac_point_shadow(Point_light light, vec3 light_dir);
float cac_spot_shadow(Spot_light light, vec3 light_dir);
float cac_direction_shadow(Direction_light light, int index);

float get_viewdepth(float fdepth, float near, float far);
float linearize_depth(float fdepth, float near, float far);
vec3 gamma_correct(vec3 color);

// global variables
const float PI = 3.14159265359;
vec4 position_buffer_sample = texture(position_buffer, texture_coord);
vec3 frag_pos = position_buffer_sample.rgb;
float frag_alpha = position_buffer_sample.a;
vec4 frag_normal_depth = texture(normal_depth, texture_coord);
vec3 frag_normal = frag_normal_depth.rgb;
float z_screen = frag_normal_depth.a;
vec3 frag_surface_normal = texture(surface_normal, texture_coord).rgb;
vec4 frag_albedo_specular = texture(albedo_specular, texture_coord);
vec3 frag_albedo = frag_albedo_specular.rgb;
float frag_roughness = frag_albedo_specular.a;
vec4 frag_ambient_metalic = texture(ambient_metalic, texture_coord);
vec3 frag_ambient = frag_ambient_metalic.rgb;
float frag_metalic = frag_ambient_metalic.a;
float occulsion = texture(ssao_buffer, texture_coord).r;

void main()
{
    if(frag_alpha <= 0.0001f) discard;
    vec3 result = vec3(0.f);
    for(int i = 0; i < point_lights_count; ++i)
        result += cac_point_light(point_lights[i]);
    for(int i = 0; i < direction_lights_count; ++i)
        result += cac_direction_light(direction_lights[i], i);
    for(int i = 0; i < spot_lights_count; ++i)
        result += cac_spot_light(spot_lights[i]);
    result += cac_env_irradiance(skybox);
    result -= ssao ? (1.f - occulsion) * 0.15f : 0.f;
    result = gamma_correct(result);
    FragColor = vec4(result, 1.f);
    if(dot(result, vec3(0.2126f, 0.7152f, 0.0722f)) < threshold) BrightColor = vec4(0.f);
    else BrightColor = vec4(result, 1.f);
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

vec3 cac_point_light(Point_light light)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 view_dir = normalize(view_pos -frag_pos);;
    vec3 h = normalize(view_dir + light_dir);
    if(dot(view_dir, frag_surface_normal) < 0 || dot(light_dir, frag_surface_normal) < 0) return vec3(0.f);

    float n_dot_l = max(dot(frag_normal, light_dir), 0);  // cosTheta

    float r = length(light.position - frag_pos);
    float F_att = 1.f / (light.kc + light.kl * r + light.kq * r * r);
    vec3 radiance = light.color * F_att * light.intensity;  // Li(p, wi)

    vec3 F0 = mix(vec3(0.04f), frag_albedo, frag_metalic);  // surface reflection at zero incidence
    float n_dot_v = max(dot(frag_normal, view_dir), 0.f);
    vec3 F = fresnelSchlick(F0, max(dot(frag_surface_normal, view_dir), 0.f));  // F
    
    float NDF = DistributionGGX(frag_normal, h, frag_roughness);  // N

    float G = GeometrySmith(frag_normal, view_dir, light_dir, frag_roughness); // G
    
    vec3 kd = (vec3(1.f) - F) * (1 - frag_metalic);
    vec3 L_oi = (kd * frag_albedo / PI + F * NDF * G / max(4 * n_dot_l * n_dot_v, 0.0001f)) * radiance * n_dot_l;
    float in_shadow = cac_point_shadow(light, light_dir);
    return L_oi * (1.f - in_shadow);
}

vec3 cac_spot_light(Spot_light light)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 h = normalize(view_dir + light_dir);
    if(dot(view_dir, frag_surface_normal) < 0 || dot(light_dir, frag_surface_normal) < 0) return vec3(0.f);
    
    float n_dot_l = max(dot(frag_normal, light_dir), 0);  // cosTheta
    
    float r = length(light.position - frag_pos);
    float F_att = 1.f / (light.kc + light.kl * r + light.kq * r * r);
    vec3 radiance = light.color * F_att * light.intensity;  // Li(p, wi)

    vec3 F0 = mix(vec3(0.04f), frag_albedo, frag_metalic);  // surface reflection at zero incidence
    float n_dot_v = max(dot(frag_normal, view_dir), 0.f);
    vec3 F = fresnelSchlick(F0, max(dot(frag_surface_normal, view_dir), 0.f));  // F
    
    float NDF = DistributionGGX(frag_normal, h, frag_roughness);  // N

    float G = GeometrySmith(frag_normal, view_dir, light_dir, frag_roughness); // G
    
    vec3 kd = (vec3(1.f) - F) * (1 - frag_metalic);
    vec3 L_oi = (kd * frag_albedo / PI + F * NDF * G / max(4 * n_dot_l * n_dot_v, 0.0001f)) * radiance * n_dot_l;
    float in_shadow = cac_spot_shadow(light, light_dir);
    
    float theta = dot(-light_dir, light.direction), epslion = light.cutoff - light.outer_cutoff;
    float opacity = clamp((theta - light.outer_cutoff) / epslion, 0.f, 1.f);
    return L_oi * (1.f - in_shadow) * opacity;
}

vec3 cac_direction_light(Direction_light light, int index)
{
    vec3 light_dir = normalize(vec3(-light.direction));
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 h = normalize(view_dir + light_dir);
    if(dot(view_dir, frag_surface_normal) < 0 || dot(light_dir, frag_surface_normal) < 0) return vec3(0.f);

    float n_dot_l = max(dot(frag_normal, light_dir), 0);  // cosTheta

    vec3 radiance = light.color * light.intensity;  // Li(p, wi)

    vec3 F0 = mix(vec3(0.04f), frag_albedo, frag_metalic);  // surface reflection at zero incidence
    float n_dot_v = max(dot(frag_normal, view_dir), 0.f);
    vec3 F = fresnelSchlick(F0, max(dot(frag_surface_normal, view_dir), 0.f));  // F
    
    float NDF = DistributionGGX(frag_normal, h, frag_roughness);  // N

    float G = GeometrySmith(frag_normal, view_dir, light_dir, frag_roughness); // G
    
    vec3 kd = (vec3(1.f) - F) * (1 - frag_metalic);
    vec3 L_oi = (kd * frag_albedo / PI + F * NDF * G / max(4 * n_dot_l * n_dot_v, 0.0001f)) * radiance * n_dot_l;
    float in_shadow = cac_direction_shadow(light, index);
    return L_oi * (1.f - in_shadow);
}

vec3 cac_env_irradiance(Skybox skybox)
{
    if(!skybox.use) return vec3(0.f);
    if(!skybox.affect_scene) return vec3(0.f);

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-view_dir, frag_normal);

    vec3 F0 = mix(vec3(0.04f), frag_albedo, frag_metalic);
    vec3 F = fresnelSchlickRoughness(F0, max(dot(frag_surface_normal, view_dir), 0.f), frag_roughness);
    vec3 kd = (1.f - F) * (1 - frag_metalic);  // nullify diffuse part of metalic surface
    
    vec3 diffuse_irradiance = texture(skybox.irradiance_cubemap, frag_normal).rgb * frag_albedo * kd;
    
    const float max_mip_level = 7.0f;
    vec3 prefilterd_specular = textureLod(skybox.prefiltered_cubemap, reflect_dir, frag_roughness * max_mip_level).rgb;
    vec2 scale_bias = texture(skybox.BRDF_LUT, vec2(max(dot(frag_normal, view_dir), 0.f), frag_roughness)).rg;
    vec3 specular_irradiance = prefilterd_specular * (F * scale_bias.x + scale_bias.y);

    vec3 irradiance = (diffuse_irradiance + specular_irradiance) * skybox.intensity * frag_ambient;
    return irradiance;
}

vec3 offset_vecs[20] = vec3[] 
    (
        vec3(-1.f, -1.f, -1.f), vec3(-1.f, -1.f, 0.f), vec3(-1.f, -1.f, 1.f), vec3(-1.f, 0.f, -1.f), vec3(-1.f, 0.f, 1.f), vec3(-1.f, 1.f, -1.f), vec3(-1.f, 1.f, 0.f), vec3(-1.f, 1.f, 1.f),
        vec3(0.f, -1.f, -1.f), vec3(0.f, -1.f, 1.f), vec3(0.f, 1.f, -1.f), vec3(0.f, 1.f, 1.f),
        vec3(1.f, -1.f, -1.f), vec3(1.f, -1.f, 0.f), vec3(1.f, -1.f, -1.f), vec3(1.f, 0.f, -1.f), vec3(1.f, 0.f, 1.f), vec3(1.f, 1.f, -1.f), vec3(1.f, 1.f, 0.f), vec3(1.f, 1.f, 1.f)
    );

float cac_point_shadow(Point_light light, vec3 light_dir)
{
    float bias_ = max(bias * (1 - dot(light_dir, frag_surface_normal)), bias * 0.01f);
    float in_shadow = 0.f;

    float radius = (1.f + linearize_depth(gl_FragCoord.z, near, far)) / 75.f;
    vec3 light2frag = frag_pos - light.position;
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
    float bias_ = max(bias * (1.f - dot(frag_surface_normal, normalize(-light.direction))), bias * 0.01f);
    float z_view = get_viewdepth(z_screen, near, far);
    
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
    vec4 light_space_clip = light_space_mat[index][shadow_index] * vec4(frag_pos, 1.f);
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
    float bias_ = max(bias * (1 - dot(light_dir, frag_surface_normal)), bias * 0.01f);
    float in_shadow = 0.f;

    float radius = (1.f + linearize_depth(gl_FragCoord.z, near, far)) / 75.f;
    vec3 light2frag = frag_pos - light.position;
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

float get_viewdepth(float fdepth, float near, float far)
{
    float z_ndc = fdepth * 2 - 1;
    return 2 * near * far/ (far + near - z_ndc * (far - near));
}

float linearize_depth(float fdepth, float near, float far)
{
    float z_ndc = fdepth * 2 - 1;
    float z_view = 2 * near * far/ (far + near - z_ndc * (far - near));
    return (z_view - near) / (far - near); 
}

vec3 gamma_correct(vec3 color)
{
    return pow(color, vec3(1.f / 2.2f));
}
