#version 460 core
in GS_OUT
{
    vec2 texture_coord;  // UV space
    vec3 frag_normal;  // world space
    vec4 frag_pos;  // world space
    mat3 TBN;  // world space
} fs_in;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

struct Material
{
    sampler2D diffuse_maps[4];
    sampler2D specular_maps[4];
    sampler2D ambient_maps[4];
    sampler2D opacity_maps[4];
    sampler2D normal_maps[4];
    sampler2D displacement_maps[4];

    bool use_diffuse_map;
    bool use_specular_map;
    bool use_ambient_map;
    bool use_opacity_map;
    bool use_normal_map;
    bool use_displacement_map;

    vec3 diffuse_color;
    vec3 specular_color;
    vec3 ambient_color;
    float opacity;
    float normal_map_strength;
    int shinness;
};

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

struct Skybox
{
    samplerCube cubemap;
    float intensity;
    bool use;
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

layout(std140, Binding = 1) uniform FN
{
    float far;
    float near;
};

layout(std140, Binding = 2) uniform Light_matrices
{
    mat4 light_space_mat[8][10];
};

uniform bool bloom;
uniform float threshold;
uniform float bias;
uniform Skybox skybox;
uniform vec3 view_pos;
uniform Material material;

uniform int point_lights_count;
uniform int spot_lights_count;
uniform int direction_lights_count;

uniform Point_light point_lights[16];
uniform Spot_light spot_lights[16];
uniform Direction_light direction_lights[8];

uniform float cascade_levels[10];
uniform int cascade_count;

const float parallel_map_scale = .028f;
const float min_layer = 8.f;
const float max_layer = mix(32.f, 64.f, parallel_map_scale / 0.05f);

vec3 caculate_point_light(Point_light light, Material material);
vec3 caculate_spot_light(Spot_light light, Material material);
vec3 caculate_direction_light(Direction_light light, Material material, int index);

vec3 caculate_skybox(Skybox skybox, Material material);
float linearize_depth(float fdepth, float near, float far);
float get_viewdepth(float fdepth, float near, float far);
vec3 get_shading_normal(Material material, vec2 texture_coord);
vec2 get_parallax_mapping_texturecoord(vec2 texture_coord, Material material, vec3 view_dir);
mat3 orthogonalize_TBN(mat3 fs_TBN);
float gray_scale(vec3 color);
vec3 gamma_correct(vec3 color);

float calculate_cascade_shadow(Direction_light light, int index);
float calculate_cubemap_shadow(Point_light light, vec3 light_dir);
float calculate_cubemap_shadow(Spot_light light, vec3 light_dir);

void main()
{
    /* 
    开启深度测试的情况下，默认应当执行early depth testing，在vertex shader着色完毕后，先执行蒙版测试决定是否舍弃当前fragment，再执行深度测试决定是否舍弃当前fragment，两层过关才开始接着执行着色
    这里opengl计算得到depth value = (1/zview - 1/n) / (1/f - 1/n) = (zndc + 1) * 0.5;，这个值就是gl_FragCoord.z，
    可以通过修改gl_FragDepth来手动更改深度值，但是注意，如果设置了gl_FragDepth，那么在fragment shader运行之前，
    这个fragment的depth是多少就不知道了，只能等fragment shader运行完才知道depth值，再进行depth test，也就是说只能
    先着色再深度测试，early depth testing就没有用了
    */
    // 所有位置相关向量和方向相关向量都都是在world space下的
    if(material.opacity < 0.0001f 
    || (material.use_opacity_map && texture(material.opacity_maps[0], fs_in.texture_coord).r < 0.0001f)) 
        discard;

    vec3 result = vec3(0.f);
    vec3 ambient = vec3(0.f);  // Ambient is isolated form specular and diffuse
    for(int i = 0; i < direction_lights_count; ++i)
    {
        ambient += direction_lights[i].ambient * direction_lights[i].color * (material.use_ambient_map ? vec3(texture(material.ambient_maps[0], fs_in.texture_coord)) : material.ambient_color) / direction_lights_count;
        result += caculate_direction_light(direction_lights[i], material, i);
    }
    for(int i = 0; i < point_lights_count; ++i)
    {
        ambient += point_lights[i].ambient * point_lights[i].color * (material.use_ambient_map ? vec3(texture(material.ambient_maps[0], fs_in.texture_coord)) : material.ambient_color) / point_lights_count;
        result += caculate_point_light(point_lights[i], material);
    }
    for(int i = 0; i < spot_lights_count; ++i)
    {
        ambient += spot_lights[i].ambient * spot_lights[i].color * (material.use_ambient_map ? vec3(texture(material.ambient_maps[0], fs_in.texture_coord)) : material.ambient_color) / spot_lights_count;
        result += caculate_spot_light(spot_lights[i], material);
    }
    result += ambient;
    result += caculate_skybox(skybox, material);
    
    float alpha = material.use_opacity_map ? texture(material.opacity_maps[0], fs_in.texture_coord).r : material.opacity;
    result = gamma_correct(result);
    FragColor = vec4(result, alpha);
    /* 需要提一句，blend function发生在fragment shader计算一个像素的FragColor之后和一个vec4写入frame buffer
    之前，也就是说如果开启了blend，在FragColor之后还有一次计算
    */
    if(bloom)
    {
        float brightness = dot(result, vec3(0.2126f, 0.7152f, 0.0722f));
        if(brightness > threshold) BrightColor = vec4(result, 1.f);
        else BrightColor = vec4(0.f, 0.f, 0.f, 0.f);
    }
}

vec3 caculate_point_light(Point_light light, Material material)
{
    vec3 view_dir = normalize(view_pos - vec3(fs_in.frag_pos));
    vec3 light_dir = normalize(light.position - vec3(fs_in.frag_pos));
    vec2 texture_coord = get_parallax_mapping_texturecoord(fs_in.texture_coord, material, view_dir);
    vec3 normal = get_shading_normal(material, texture_coord);
    if(dot(view_dir, normalize(fs_in.frag_normal)) < 0 || dot(light_dir, normalize(fs_in.frag_normal)) < 0) return vec3(0.f);

    float r = length(light.position - vec3(fs_in.frag_pos));
    float F_att = 1.f / (light.kc + light.kl * r + light.kq * r * r);
    float n_shadow = calculate_cubemap_shadow(light, light_dir);

    vec3 h = normalize(view_dir + light_dir);
    float ks = material.use_specular_map ? texture(material.specular_maps[0], texture_coord).r : gray_scale(material.specular_color);
    vec3 kd = material.use_diffuse_map ? vec3(texture(material.diffuse_maps[0], texture_coord)) : material.diffuse_color;
    vec3 diffuse = kd * light.intensity * max(dot(light_dir, normal), 0.f) * light.color * light.diffuse;
    vec3 specular = ks * light.intensity * pow(max(dot(h, normal), 0.f), 32) * light.color * light.specular;
    return (diffuse + specular) * n_shadow * F_att;
}

vec3 caculate_spot_light(Spot_light light, Material material)
{
    vec3 view_dir = normalize(view_pos - vec3(fs_in.frag_pos));
    vec3 light_dir = normalize(light.position - vec3(fs_in.frag_pos));
    vec2 texture_coord = get_parallax_mapping_texturecoord(fs_in.texture_coord, material, view_dir);
    vec3 normal = get_shading_normal(material, texture_coord);
    if(dot(view_dir, normalize(fs_in.frag_normal)) < 0 || dot(light_dir, normalize(fs_in.frag_normal)) < 0) return vec3(0.f);
    float theta = dot(-light_dir, light.direction), epslion = light.cutoff - light.outer_cutoff;
    float opacity = clamp((theta - light.outer_cutoff) / epslion, 0.0f, 1.f);
    float r = length(light.position - vec3(fs_in.frag_pos));
    float F_att = 1.f / (light.kc + light.kl * r + light.kq * r * r);
    float n_shadow = calculate_cubemap_shadow(light, light_dir);

    vec3 h = normalize(view_dir + light_dir);
    float ks = material.use_specular_map ? texture(material.specular_maps[0], texture_coord).r : gray_scale(material.specular_color);
    vec3 kd = material.use_diffuse_map ? vec3(texture(material.diffuse_maps[0], texture_coord)) : material.diffuse_color;
    vec3 diffuse = kd * light.intensity * max(dot(light_dir, normal), 0.f) * light.color * light.diffuse;
    vec3 specular = ks * light.intensity * pow(max(dot(h, normal), 0.f), 32) * light.color * light.specular;
    return (diffuse + specular) * opacity * n_shadow * F_att;
}

vec3 caculate_direction_light(Direction_light light, Material material, int index)
{
    vec3 view_dir = normalize(view_pos - vec3(fs_in.frag_pos));
    vec3 light_dir = normalize(vec3(-light.direction));
    vec2 texture_coord = get_parallax_mapping_texturecoord(fs_in.texture_coord, material, view_dir);
    vec3 normal = get_shading_normal(material, texture_coord);
    if(dot(view_dir, normalize(fs_in.frag_normal)) < 0 || dot(light_dir, normalize(fs_in.frag_normal)) < 0) return vec3(0.f);
    float n_shadow = calculate_cascade_shadow(light, index);

    vec3 h = normalize(view_dir + light_dir);
    float ks = material.use_specular_map ? texture(material.specular_maps[0], texture_coord).r : gray_scale(material.specular_color);
    vec3 kd = material.use_diffuse_map ? vec3(texture(material.diffuse_maps[0], texture_coord)) : material.diffuse_color;
    vec3 diffuse = kd * light.intensity * max(dot(light_dir, normal), 0.f) * light.color * light.diffuse;
    vec3 specular = ks * light.intensity * pow(max(dot(h, normal), 0.f), 32) * light.color * light.specular;
    return (diffuse + specular) * n_shadow;
}

vec3 caculate_skybox(Skybox skybox, Material material)
{
    if(!skybox.use) return vec3(0.f);
    vec3 eye_dir = normalize(vec3(fs_in.frag_pos) - view_pos);
    vec2 texture_coord = get_parallax_mapping_texturecoord(fs_in.texture_coord, material, -eye_dir);
    vec3 normal = get_shading_normal(material, texture_coord);
    vec3 relfect_dir = reflect(eye_dir, normal);
    
    vec3 kd = material.use_diffuse_map ? vec3(texture(material.diffuse_maps[0], texture_coord)) : material.diffuse_color;
    float coffi = max(dot(-eye_dir, fs_in.frag_normal), 0);
    return vec3(texture(skybox.cubemap, relfect_dir)) * kd * skybox.intensity * coffi;
}

float linearize_depth(float fdepth, float near, float far)
{
    float z_ndc = fdepth * 2 - 1;
    float z_view = 2 * near * far/ (far + near - z_ndc * (far - near));
    return (z_view - near) / (far - near); 
}

float get_viewdepth(float fdepth, float near, float far)
{
    float z_ndc = fdepth * 2 - 1;
    return 2 * near * far/ (far + near - z_ndc * (far - near));
}

float calculate_cascade_shadow(Direction_light light, int index)
{
    float n_shadow = 0.f;
    float bias_ = max(bias * (1 - dot(normalize(-light.direction), normalize(fs_in.frag_normal))), bias * 0.01f);
    // select cascade depthmap
    float z_view = get_viewdepth(gl_FragCoord.z, near, far);

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

    vec4 frag_pos_light_space = light_space_mat[index][shadow_index] * fs_in.frag_pos;
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
    float bias_ = max(bias * (1 - dot(light_dir, normalize(fs_in.frag_normal))), bias * 0.01f);
    vec3 light2frag = vec3(fs_in.frag_pos) - light.position;
    float depth_current = length(light2frag);
    float max_depth = length(light2frag * min(light.far / abs(light2frag.x), min(light.far / abs(light2frag.y), light.far / abs(light2frag.z))));
    float min_depth = length(light2frag * min(light.near / abs(light2frag.x), min(light.near / abs(light2frag.y), light.near / abs(light2frag.z))));

    float radius = (1.f + linearize_depth(gl_FragCoord.z, near, far)) / 75.f;
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
    float bias_ = max(bias * (1 - dot(light_dir, normalize(fs_in.frag_normal))), bias * 0.01f);
    vec3 light2frag = vec3(fs_in.frag_pos) - light.position;
    float depth_current = length(light2frag);
    float max_depth = length(light2frag * min(light.far / abs(light2frag.x), min(light.far / abs(light2frag.y), light.far / abs(light2frag.z))));
    float min_depth = length(light2frag * min(light.near / abs(light2frag.x), min(light.near / abs(light2frag.y), light.near / abs(light2frag.z))));

    float radius = (1.f + linearize_depth(gl_FragCoord.z, near, far)) / 75.f;
    for(int i = 0; i < 20; ++i)
    {
        float depth_closest = texture(light.depth_cubemap, light2frag + radius * offset_vecs[i]).r;
        depth_closest = depth_closest * (max_depth - min_depth) + min_depth;
        n_shadow += (depth_closest + bias_) > depth_current ? 1.f : 0.f;
    }
    n_shadow /= 20.f;
    return n_shadow;
}

vec3 get_shading_normal(Material material, vec2 texture_coord)
{
    vec3 normal = material.use_normal_map ? normalize(fs_in.TBN * (texture(material.normal_maps[0], texture_coord).rgb * 2 - 1)) : normalize(fs_in.frag_normal);
    if(material.use_normal_map)
        normal = normalize(material.normal_map_strength * normal + (1 - material.normal_map_strength) * normalize(fs_in.frag_normal));
    return normal;
}

vec2 get_parallax_mapping_texturecoord(vec2 texture_coord, Material material, vec3 view_dir)
{
    if(!material.use_displacement_map || parallel_map_scale == 0.f) return texture_coord;
    // view dir is in world space, transforming it to tangent space first
    vec3 view_dir_tangent = normalize(transpose(fs_in.TBN) * view_dir);
    float num_layers = mix(max_layer, min_layer, max(dot(vec3(0.f, 0.f, 1.f), view_dir_tangent), 0.f));
    float layer_depth = parallel_map_scale / num_layers;
    float current_layer_depth = 0.f;
    vec2 current_texture_coords = texture_coord;
    vec2 delta_texture_coords = view_dir_tangent.xy * parallel_map_scale / (num_layers * view_dir_tangent.z);
    float current_depth_value = texture(material.displacement_maps[0], current_texture_coords).r * parallel_map_scale;
    while(current_layer_depth < current_depth_value)
    {
        current_texture_coords -= delta_texture_coords;
        current_layer_depth += layer_depth;
        current_depth_value = texture(material.displacement_maps[0], current_texture_coords).r * parallel_map_scale;
    }
    
    vec2 prev_texture_coords = current_texture_coords + delta_texture_coords;
    float depth_after = abs(current_layer_depth - current_depth_value);
    float depth_before = abs(texture(material.displacement_maps[0], prev_texture_coords).r * parallel_map_scale - current_layer_depth + layer_depth);
    float weight = depth_after / (depth_after + depth_before);

    return prev_texture_coords * weight + (1.f - weight) * current_texture_coords;
}

mat3 orthogonalize_TBN(mat3 fs_TBN)
{
    vec3 T = fs_TBN[0]; 
    vec3 B = fs_TBN[1]; 
    vec3 N = fs_TBN[2];
    T = T - dot(N, T) * N;
    B = cross(N, T);
    return mat3(T, B, N);
}

float gray_scale(vec3 color)
{
    return 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
}

vec3 gamma_correct(vec3 color)
{
    return pow(color, vec3(1.f / 2.2f));
}