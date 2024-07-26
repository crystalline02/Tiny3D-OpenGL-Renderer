#version 460 core
layout(location = 0) out vec4 position_buffer;
layout(location = 1) out vec4 normal_depth;
layout(location = 2) out vec4 surface_normal;
layout(location = 3) out vec4 albedo_specular;
layout(location = 4) out vec4 ambient_metalic;

in GS_OUT
{
    vec4 frag_position;  // world
    mat3 TBN;  // world
    vec3 frag_normal;  // world
    vec2 texture_coord;
} fs_in;

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

uniform Material material;
uniform vec3 view_pos;

const float parallel_map_scale = .028f;
const float min_layer = 8.f;
const float max_layer = mix(32.f, 64.f, parallel_map_scale / 0.05f);

vec2 get_parallax_mapping_texturecoord(vec2 texture_coord, Material material, vec3 view_dir);

void main()
{
    // compute texture coordinates for parallax mapping
    vec3 viewDir = normalize(view_pos - vec3(fs_in.frag_position));
    vec2 texture_coord = get_parallax_mapping_texturecoord(fs_in.texture_coord, material, viewDir);
    
    // Perform alpha clip
    float alpha = material.use_opacity_map ? texture(material.opacity_maps[0], texture_coord).r : material.opacity;
    if(alpha < 1e-4) discard;

    position_buffer = vec4(fs_in.frag_position.xyz, alpha);  // world

    vec3 normal = material.use_normal_map ? normalize(fs_in.TBN * (texture(material.normal_maps[0], texture_coord).rgb * 2.f - 1.f)) : normalize(fs_in.frag_normal);
    if(material.use_normal_map)
        normal = normalize(normal * material.normal_map_strength + fs_in.TBN[2] * (1.f - material.normal_map_strength));
    normal_depth = vec4(normal, float(gl_FragCoord.z));  // world

    surface_normal = vec4(normalize(fs_in.frag_normal), 1.f);

    albedo_specular = vec4(material.use_diffuse_map ? texture(material.diffuse_maps[0], texture_coord).rgb : material.diffuse_color, 
        material.use_specular_map ? texture(material.specular_maps[0], texture_coord).r : 0.299f * material.specular_color.r + 0.587f * material.specular_color.g + 0.114f * material.specular_color.b);
    
    ambient_metalic = vec4(material.use_ambient_map ? texture(material.ambient_maps[0], texture_coord).rgb : material.ambient_color, 1.f);
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