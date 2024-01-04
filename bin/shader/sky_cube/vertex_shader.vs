# version 460 core
layout(location = 0) in vec3 aPos;

out vec3 texture_coords;

uniform mat4 view;
uniform mat4 projection;
// model = I, no need to pass that matrix

void main()
{
    texture_coords = aPos;
    // 如果取gl_Position为.xyww，是因为经过透视除法后，顶点的z值必然为1.f，也就是说zndc = 1.f，那么F_depth = (zndc + 1) * 0.5 = 1.f，
    // 于是深度测试在有绘制物体的地方总是不通过，cubemap总是被画在最后面，达到了我们的预期效果
    gl_Position = (projection * view * vec4(aPos, 1.f)).xyww;
}