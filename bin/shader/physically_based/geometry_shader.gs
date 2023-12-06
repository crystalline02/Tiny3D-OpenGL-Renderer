#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT
{
    vec3 normal;  // world
    vec3 tangent;  // world
    vec2 texture_coord;  // UV
} gs_in[3];

out GS_OUT
{
    vec3 frag_normal;  // world
    vec4 frag_pos;  // world
    vec2 texture_coord; // UV
    mat3 TBN;
} gs_out;

layout(std140, Binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

void main()
{
    for(int i = 0; i < 3; ++i)
    {
        gl_Position = projection * view * gl_in[i].gl_Position;  // clip
        gs_out.frag_normal = gs_in[i].normal;  // world
        gs_out.frag_pos = gl_in[i].gl_Position;  // world
        gs_out.texture_coord = gs_in[i].texture_coord;  // UV
        gs_out.TBN = mat3(normalize(gs_in[i].tangent), 
            normalize(cross(normalize(gs_in[i].tangent), normalize(gs_in[i].normal))),
            normalize(gs_in[i].normal));
        EmitVertex();
    }
    EndPrimitive();
}