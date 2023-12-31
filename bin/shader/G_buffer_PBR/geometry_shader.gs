#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT
{
    vec3 normal;  // world space
    vec3 tangent;  // world space
    vec2 texture_coord;
} gs_in[];

out GS_OUT
{
    vec4 frag_position;  // world
    mat3 TBN;  // world
    vec3 frag_normal;  // world
    vec2 texture_coord;
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
        gs_out.frag_position = gl_in[i].gl_Position;  // world
        gs_out.TBN = mat3(gs_in[i].tangent,
            normalize(cross(gs_in[i].tangent, gs_in[i].normal)),
            gs_in[i].normal);
        gs_out.frag_normal = gs_in[i].normal;
        gs_out.texture_coord = gs_in[i].texture_coord;
        gl_Position = projection *  view * gl_in[i].gl_Position; // clip space
        EmitVertex();
    }
    EndPrimitive();
}