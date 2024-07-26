# version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VSOut
{
    vec3 normal;
    vec3 tangent;
    vec2 textureCoord;
} gsIn[3];

out GSOut
{
    vec3 fragNormal;
    vec4 fragPosition;
    mat3 TBN;
    vec2 textureCoord;
} gsOut;

layout(std140, Binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
};

void main()
{
    for(int i = 0; i < 3; ++i)
    {
        gl_Position = projection * view * gl_in[i].gl_Position;
        gsOut.fragPosition = gl_in[i].gl_Position;  // world;
        gsOut.fragNormal = normalize(gsIn[i].normal);  // world;
        gsOut.textureCoord = gsIn[i].textureCoord;  // uv
        gsOut.TBN = mat3(
            normalize(gsIn[i].tangent),
            normalize(cross(gsIn[i].tangent, gsIn[i].normal)),
            normalize(gsIn[i].normal)
        );
        EmitVertex();
    }
    EndPrimitive();
}