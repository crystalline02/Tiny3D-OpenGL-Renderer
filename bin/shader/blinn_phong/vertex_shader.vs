#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTextureCoord;

// 从vertex shader out到fragment shader中的值都会做透视矫正的三角形重心坐标插值
// 我们应当理解为，vertex shader输出的属性都是顶点自身的属性，不应该是和顶点无关的属性

out VS_OUT
{
    vec3 normal;  // world space
	vec3 tangent;  // world space
    vec2 texture_coord;  // UV space
} vs_out;

uniform mat4 model;
uniform mat3 normal_mat;

void main()
{
    /* 
	注意！我们在使用glm::perpective获得的透视矩阵，实际上是：M_orth * M_presp得到的，并不是只有一个M_presp，
	所以经过了 projection * view * model变换后，gl_Position再被透视除法，这时，gl_Position已经在NDC下了[-1,1]^3！

	然后这个坐标再缩放平移移到screen space下（由glViewport指定自动完成）；
	在整个过程中可以保证坐标的w分量就是z值，最后Fragment shader就针对在screen space下的顶点（仍然是一个vec4），做插值并渲染
	我在笔记上有写出每一步计算的坐标的表达式

	另外，提一提表达式，经过M_presp得到的坐标为[nx, ny, z(n+f)-nf, z]（这里xyz表示顶点在view space下的坐标，我们从view space开始看）
	再做平移缩放，经过M_orth后坐标为[/frac{x}{ratio*tan(halffov)}, /frac{y}{tan(halffov)}, /frac{z(n+f)}{n-f} - /frac{2nf}{(f-n)}, z]，这就是NDC下[-1, 1]^3（自动计算完成）的坐标，也即此处的gl_Positon；
	然后在自动执行透视除法，得到坐标[/frac{x}{z*ratio*tan(halffov)}, /frac{y}{z*tan(halffov)}, /frac{(n+f)}{n-f} - /frac{2nf}{z(f-n)},z]；
	*/
    gl_Position = model * vec4(aPos, 1.f);  // local space -> world space
    vs_out.normal = normalize(normal_mat * aNormal);  // local space -> world space
	vs_out.tangent = normalize(normal_mat * aTangent);  // local space -> world sapce
    vs_out.texture_coord = aTextureCoord;  // UV space
}