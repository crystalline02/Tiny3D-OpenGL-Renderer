#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

/*
in gl_vertex
{
    vec3 gl_Position;  // 注：geometry shader接收的gl_Poisiton并为经过透视除法
    float gl_PointSize;
    float gl_ClipDistance[];
} gl_in[];
*/

// Vertex shader output, an array for a primitive(per vertex)
in VS_OUT
{
    vec3 normal;  // world space
    vec3 tangent;  // world space
    vec2 texture_coord;  // UV space
} gs_in[];

// Geometry shader output, per vertex for a primitive, not an array
out GS_OUT
{
    vec2 texture_coord;  // UV space
    vec3 frag_normal;  // world space
    vec4 frag_pos;  // world space
    mat3 TBN;  // world space
} gs_out;

// 下面是一个Uniform block，数据在内存的分布方式是符合std140规则的
layout(std140, Binding = 0) uniform Matrices
{
    mat4 view;  // base alignment: 16 * 4 = 64, alignment offset: 0 
    mat4 projection;  // base alignment: 16 * 4 = 64, alignment offset: 64
};

void main()
{
    // gl_Position is in world space
    for(int i = 0; i < 3; ++i)
    {
        gs_out.TBN = mat3(normalize(gs_in[i].tangent), 
            normalize(cross(gs_in[i].tangent, gs_in[i].normal)),  // B = cross(N, T) or B = cross(T, N)
            normalize(gs_in[i].normal));
        gs_out.texture_coord = gs_in[i].texture_coord;  // UV
        gs_out.frag_normal = gs_in[i].normal;  // world space
        gs_out.frag_pos = gl_in[i].gl_Position;  //  world space
        gl_Position = projection * view * gl_in[i].gl_Position;  // clip space
        /*
        我在这里不能先前对gl_Position做透视除法，也就是不能gl_Position = gl_Position / gl_Position.w，这里解释一下为什么
        fragment shader中获得的所有从geometry shader输出的变量，如texture_coord，frag_pos，frag_normal都是通过三角形插值得到的，回想一下透视矫正的插值公式，为：
        I_interpolate = (I[0] * alpha / z[0] + I[1] * beta / z[1] + I[2] * gamma / z[2]) / (alpha / z[0] + beta / z[1] + gamma / z[2])
        这个公式中用到的z值就是z_view，z_view从何而来？从gl_Position的w分量而来！如果你提前做透视矫正，那么gl_Position.w = 1.f，这样插值出来的结果就不对了
        */
        EmitVertex();
    }
    EndPrimitive();
}