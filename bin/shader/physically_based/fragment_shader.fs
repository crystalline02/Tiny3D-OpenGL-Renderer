#version 460 core

in GS_OUT
{
    vec3 frag_normal;  // world
    vec3 frag_pos;  // world
    vec2 texture_coord; // UV
    mat3 TBN;  // world
} fs_in;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

struct Material
{
    sampler2D albedo_maps[4];
    sampler2D specular_maps[4];
    sampler2D opacity_maps[4];
    sampler2D normal_maps[4];
    sampler2D displacement_maps[4];
    sampler2D metalic_maps[4];
    
    bool use_albedo_map;
    bool use_opacity_map;
    bool use_normal_map;
    bool use_displacement_map;
    bool use_metalic_map;

    vec3 albedo_color;
    vec3 specular_color;
    float opacity;
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

uniform Material material;
uniform Skybox skybox;


void main()
{
    FragColor = vec4(1.f, 1., 0.f, 1.f);
}