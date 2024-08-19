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

layout(std140, Binding = 3) uniform JitterVec
{
    vec2 jitter;
};

layout(std140, Binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

layout(std140, Binding = 4) uniform 

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
        vec4 clipPos = projection *  view * gl_in[i].gl_Position; // clip space
        gl_Position = clipPos + vec4(jitter, 0.f, 0.f) * clipPos.w;  // jitter in clip space
        EmitVertex();
    }
    EndPrimitive();
}