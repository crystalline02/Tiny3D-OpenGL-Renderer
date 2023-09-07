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
    samplerCube cubemap;
    float intensity;
    bool use;
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
uniform sampler2D ambient_buffer;
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

vec3 caculate_point_light(Point_light light);
vec3 caculate_spot_light(Spot_light light);
vec3 caculate_direction_light(Direction_light light, int index);
vec3 caculate_skybox(Skybox skybox);
float calculate_cascade_shadow(Direction_light light, int index);
float calculate_cubemap_shadow(Point_light light, vec3 light_dir);
float calculate_cubemap_shadow(Spot_light light, vec3 light_dir);
float get_viewdepth(float fdepth, float near, float far);
float linearize_depth(float fdepth, float near, float far);

// global variables
vec4 position_buffer_sample = texture(position_buffer, texture_coord);
vec3 frag_pos = position_buffer_sample.rgb;
float frag_alpha = position_buffer_sample.a;
vec4 frag_normal_depth = texture(normal_depth, texture_coord);
vec3 frag_normal = frag_normal_depth.rgb;
float z_screen = frag_normal_depth.a;
vec3 frag_surface_normal = texture(surface_normal, texture_coord).rgb;
vec4 frag_albedo_specular = texture(albedo_specular, texture_coord);
vec3 frag_albedo = frag_albedo_specular.rgb;
float frag_specular = frag_albedo_specular.a;
vec3 frag_ambient = texture(ambient_buffer, texture_coord).rgb;
float occulsion = texture(ssao_buffer, texture_coord).r;

void main()
{
    if(frag_alpha <= 0.0001f) discard;
    vec3 result = vec3(0.f), ambient = vec3(0.f);
    for(int i = 0; i < point_lights_count; ++i)
    {
        ambient += point_lights[i].ambient * point_lights[i].color * frag_ambient / point_lights_count;
        result += caculate_point_light(point_lights[i]);
    }
    for(int i = 0; i < direction_lights_count; ++i)
    {
        ambient += direction_lights[i].ambient * direction_lights[i].color * frag_ambient / direction_lights_count;
        result += caculate_direction_light(direction_lights[i], i);
    }
    for(int i = 0; i < spot_lights_count; ++i)
    {
        ambient += spot_lights[i].ambient * spot_lights[i].color * frag_ambient / spot_lights_count;
        result += caculate_spot_light(spot_lights[i]);
    } 
    result += ambient;
    result += caculate_skybox(skybox);
    result -= ssao ? (1.f - occulsion) * 0.15f : 0.f;
    FragColor = vec4(result, 1.f);
    if(dot(result, vec3(0.2126f, 0.7152f, 0.0722f)) < threshold) BrightColor = vec4(0.f);
    else BrightColor = vec4(result, 1.f);
}

vec3 caculate_point_light(Point_light light)
{
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 light_dir = normalize(light.position - frag_pos);
    if(dot(view_dir, frag_surface_normal) < 0 || dot(light_dir, frag_surface_normal) < 0) return vec3(0.f);
    float r = length(light.position - frag_pos);
    float F_att = 1.f / (light.kc + light.kl * r + light.kq * r * r);
    float n_shadow = calculate_cubemap_shadow(light, light_dir);
    vec3 h = normalize(view_dir + light_dir);
    float ks = frag_specular;
    vec3 kd = frag_albedo;

    vec3 diffuse = kd * light.intensity * max(dot(light_dir, frag_normal), 0.f) * light.color * light.diffuse;
    vec3 specular = ks * light.intensity * pow(max(dot(h, frag_normal), 0.f), 32) * light.color * light.specular;
    return (diffuse + specular) * n_shadow * F_att;
}

vec3 caculate_spot_light(Spot_light light)
{
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 light_dir = normalize(light.position - frag_pos);
    if(dot(view_dir, frag_surface_normal) < 0 || dot(light_dir, frag_surface_normal) < 0) return vec3(0.f);
    float theta = dot(-light_dir, light.direction), epslion = light.cutoff - light.outer_cutoff;
    float opacity = clamp((theta - light.outer_cutoff) / epslion, 0.0f, 1.f);
    float r = length(light.position - frag_pos);
    float F_att = 1.f / (light.kc + light.kl * r + light.kq * r * r);
    float n_shadow = calculate_cubemap_shadow(light, light_dir);
    vec3 h = normalize(view_dir + light_dir);
    float ks = frag_specular;
    vec3 kd = frag_albedo;

    vec3 diffuse = kd * light.intensity * max(dot(light_dir, frag_normal), 0.f) * light.color * light.diffuse;
    vec3 specular = ks * light.intensity * pow(max(dot(h, frag_normal), 0.f), 32) * light.color * light.specular;
    return (diffuse + specular) * opacity * n_shadow * F_att;
}

vec3 caculate_direction_light(Direction_light light, int index)
{
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 light_dir = normalize(vec3(-light.direction));
    if(dot(view_dir, frag_surface_normal) < 0 || dot(light_dir, frag_surface_normal) < 0) return vec3(0.f);
    float n_shadow = calculate_cascade_shadow(light, index);
    vec3 h = normalize(view_dir + light_dir);
    float ks = frag_specular;
    vec3 kd = frag_albedo;

    vec3 diffuse = kd * light.intensity * max(dot(light_dir, frag_normal), 0.f) * light.color * light.diffuse;
    vec3 specular = ks * light.intensity * pow(max(dot(h, frag_normal), 0.f), 32) * light.color * light.specular;
    return (diffuse + specular) * n_shadow;
}

vec3 caculate_skybox(Skybox skybox)
{
    if(!skybox.use) return vec3(0.f);
    vec3 eye_dir = normalize(frag_pos - view_pos);
    vec3 relfect_dir = reflect(eye_dir, frag_normal);
    
    vec3 kd = frag_albedo;
    float coffi = max(dot(-eye_dir, frag_surface_normal), 0);
    return vec3(texture(skybox.cubemap, relfect_dir)) * kd * skybox.intensity * coffi;
}

float calculate_cascade_shadow(Direction_light light, int index)
{
    float n_shadow = 0.f;
    float bias_ = max(bias * (1 - dot(normalize(-light.direction), normalize(frag_surface_normal))), bias * 0.01f);
    // select cascade depthmap
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

    vec4 frag_pos_light_space = light_space_mat[index][shadow_index] * vec4(frag_pos, 1.f);
    vec3 frag_pos_light_ndc = frag_pos_light_space.xyz / frag_pos_light_space.w;
    vec3 frag_pos_light_screen = frag_pos_light_ndc * 0.5f + 0.5f;
    vec2 texel_size = 1.f / vec2(textureSize(light.cascade_maps, 0));
    if(frag_pos_light_screen.z >= 1.f) return 1.f;
    float depth_current = frag_pos_light_screen.z;

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float depth_closest = texture(light.cascade_maps, 
                vec3(frag_pos_light_screen.xy + vec2(x * texel_size.x, y * texel_size.y), shadow_index)).r;
            n_shadow += ((depth_closest + bias_) > depth_current ? 1.f : 0.f);
        }
    }
    n_shadow /= 9.f;
    
    return n_shadow;
}

vec3 offset_vecs[20] = vec3[] 
    (
        vec3(-1.f, -1.f, -1.f), vec3(-1.f, -1.f, 0.f), vec3(-1.f, -1.f, 1.f), vec3(-1.f, 0.f, -1.f), vec3(-1.f, 0.f, 1.f), vec3(-1.f, 1.f, -1.f), vec3(-1.f, 1.f, 0.f), vec3(-1.f, 1.f, 1.f),
        vec3(0.f, -1.f, -1.f), vec3(0.f, -1.f, 1.f), vec3(0.f, 1.f, -1.f), vec3(0.f, 1.f, 1.f),
        vec3(1.f, -1.f, -1.f), vec3(1.f, -1.f, 0.f), vec3(1.f, -1.f, -1.f), vec3(1.f, 0.f, -1.f), vec3(1.f, 0.f, 1.f), vec3(1.f, 1.f, -1.f), vec3(1.f, 1.f, 0.f), vec3(1.f, 1.f, 1.f)
    );

float calculate_cubemap_shadow(Point_light light, vec3 light_dir)
{
    float n_shadow = 0.f;
    float bias_ = max(bias * (1 - dot(light_dir, normalize(frag_surface_normal))), bias * 0.01f);
    vec3 light2frag = frag_pos - light.position;
    float depth_current = length(light2frag);
    float max_depth = length(light2frag * min(light.far / abs(light2frag.x), min(light.far / abs(light2frag.y), light.far / abs(light2frag.z))));
    float min_depth = length(light2frag * min(light.near / abs(light2frag.x), min(light.near / abs(light2frag.y), light.near / abs(light2frag.z))));

    float radius = (1.f + linearize_depth(z_screen, near, far)) / 75.f;
    for(int i = 0; i < 20; ++i)
    {
        float depth_closest = texture(light.depth_cubemap, light2frag + radius * offset_vecs[i]).r;
        depth_closest = depth_closest * (max_depth - min_depth) + min_depth;
        n_shadow += (depth_closest + bias_) > depth_current ? 1.f : 0.f;
    }
    n_shadow /= 20.f;
    return n_shadow;
}

float calculate_cubemap_shadow(Spot_light light, vec3 light_dir)
{
    float n_shadow = 0.f;
    float bias_ = max(bias * (1 - dot(light_dir, normalize(frag_surface_normal))), bias * 0.01f);
    vec3 light2frag = frag_pos - light.position;
    float depth_current = length(light2frag);
    float max_depth = length(light2frag * min(light.far / abs(light2frag.x), min(light.far / abs(light2frag.y), light.far / abs(light2frag.z))));
    float min_depth = length(light2frag * min(light.near / abs(light2frag.x), min(light.near / abs(light2frag.y), light.near / abs(light2frag.z))));

    float radius = (1.f + linearize_depth(z_screen, near, far)) / 75.f;
    for(int i = 0; i < 20; ++i)
    {
        float depth_closest = texture(light.depth_cubemap, light2frag + radius * offset_vecs[i]).r;
        depth_closest = depth_closest * (max_depth - min_depth) + min_depth;
        n_shadow += (depth_closest + bias_) > depth_current ? 1.f : 0.f;
    }
    n_shadow /= 20.f;
    return n_shadow;
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