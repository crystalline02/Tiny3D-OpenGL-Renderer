#version 460 core
layout(triangles) in;
layout(line_strip, max_vertices = 18) out;

in VS_OUT
{
    vec3 normal;  // world space
    vec3 tangent;  // world space
} gs_in[];

layout(std140, Binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

out vec3 color;

uniform float magnitude;

void main()
{
    for(int i = 0; i < 3; ++i)
    {
        gl_Position = projection * view * gl_in[i].gl_Position;  // world space -> clip space
        color = vec3(0.f, 0.f, 1.f);
        EmitVertex();
        gl_Position = projection * view * (gl_in[i].gl_Position + vec4(gs_in[i].normal * magnitude, 0.f));
        color = vec3(0.f, 0.f, 1.f);
        EmitVertex();
        EndPrimitive();

        gl_Position = projection * view * gl_in[i].gl_Position;
        color = vec3(1.f, 0.f, 0.f);
        EmitVertex();
        gl_Position = projection * view * (gl_in[i].gl_Position + vec4(gs_in[i].tangent * magnitude, 0.f));
        color = vec3(1.f, 0.f, 0.f);
        EmitVertex();
        EndPrimitive();

        vec3 bitangent = normalize(cross(gs_in[i].normal, gs_in[i].tangent));
        gl_Position = projection * view * gl_in[i].gl_Position;
        color = vec3(0.f, 1.f, 0.f);
        EmitVertex();
        gl_Position = projection * view * (gl_in[i].gl_Position + vec4(bitangent * magnitude, 0.f));
        color = vec3(0.f, 1.f, 0.f);
        EmitVertex();
        EndPrimitive();
    }
}