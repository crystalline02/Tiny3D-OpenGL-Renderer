#version 460 core

layout(triangles, invocations = 1) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140, Binding = 2) uniform Light_matrices
{
    mat4 light_space_mat[8][10];
};

uniform int dir_light_id;

void main()
{
    gl_Layer = gl_InvocationID;
    for(int i = 0; i < 3; ++i)
    {
        gl_Position = light_space_mat[dir_light_id][gl_InvocationID] * gl_in[i].gl_Position;  // world space->light clip space
        EmitVertex(); 
    }
    EndPrimitive();
}