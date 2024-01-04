# version 460 core
in vec3 texture_coords;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;
// layout(depth_greater) out float gl_FragDepth;  // 这句话的意思是指定我们接下来赋值的gl_FragDepth的值一定会大于gl_FragCoords.z，并且保证opengl还是会做early depth testing，仅>4.2版本可用
// layout(depth_less) out float gl_FragDepth;  // 这句话的意思是指定我们接下来赋值的gl_FragDepth的值一定会小于gl_FragCoords.z，并且保证opengl还是会做early depth testing，仅>4.2版本可用
// 总的来说layout(depth_less/depth_greater)用于控制opengl深度测试的行为

uniform samplerCube skybox;
uniform float brightness;
uniform float threshold;

void main()
{
    /*
    如果设置gl_FragDepth = 1.f; 那么在fragment shader运行之前，
    这个fragment的depth是多少就不知道了，只能等fragment shader运行完才知道depth值，
    再进行depth test，early depth testing就没有用了
    */
    vec3 result = texture(skybox, texture_coords).rgb * brightness;  // if skybox is RGB16F, the sampled color vaules exceeds the [0, 1] range.
    FragColor = vec4(result, 1.f);
    if(dot(result, vec3(0.2126f, 0.7152f, 0.0722f)) < threshold) BrightColor = vec4(0.f, 0.f, 0.f, 0.f);
    else BrightColor = vec4(result, 1.f);
}