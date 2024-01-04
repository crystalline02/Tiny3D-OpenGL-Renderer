#version 460 core
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in vec3 normal[];  // world space

layout(std140, Binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

uniform float magnitude;

void main()
{
    for(int i = 0; i < 3; ++i)
    {
        gl_Position = projection * view * gl_in[i].gl_Position;
        EmitVertex();
        gl_Position = projection * view * (gl_in[i].gl_Position + vec4(normal[i] * magnitude, 0.f));
        EmitVertex();
        EndPrimitive();
    }
}