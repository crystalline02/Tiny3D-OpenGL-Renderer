#version 460 core

layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 light_space_mat[6];
out vec3 frag_pos;  // world_space

void main()
{
    /*
    提醒一下，我们有两种方式实现一次draw call 生成多张图像：
    1.一种方式是指明invocations参数，意思是这个gs会被并行地调用多次（所谓的调用一次就是指遍历gs遍历完场景中所有的图元顶点），
    gl_InvocationID这个内置遍变量可以告诉我们当前这个gs正在处理的图元顶点来自第几次调用。指定第i次调用绘制在第i个gl_Layer上，
    我们就可以实现渲染多图像，每次调用渲染一个图像。
    2.另一种方式是不指明这个invocations，也就是值调用一次gs，但是在这一次的gs调用中，我们把这个图元的顶点放到多个不同的gl_Layer上
    在交给fs来渲染图像，也能达成渲染多图像的目的。
    
    下面是第二种解决方案
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;  // 指明接下来fragment shader渲染的color填充在cubemap的哪个面
        for(int i = 0; i < 3; ++i)
        {
            gl_Position = light_space_mat[face] * gl_in[i].gl_Position;  // world space -> light space
            frag_pos = vec3(gl_in[i].gl_Position);  // world space
            EmitVertex();
        }
        EndPrimitive();
    }
    */
    
    gl_Layer = gl_InvocationID;  // 指明接下来fragment shader渲染的color填充在cubemap的哪个面
    for(int i = 0; i < 3; ++i)
    {
        gl_Position = light_space_mat[gl_InvocationID] * gl_in[i].gl_Position;  // world space -> light space
        frag_pos = vec3(gl_in[i].gl_Position);  // world space
        EmitVertex();
    }
    EndPrimitive();
}