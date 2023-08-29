# version 460 core
in vec3 texture_coords;

out vec4 FragColor;
// layout(depth_greater) out float gl_FragDepth;  // 这句话的意思是指定gl_FragDepth一定会大于gl_FragCoords.z，并且保证opengl还是会做early depth testing，仅>4.2版本可用

uniform samplerCube skybox;
uniform float brightness;

void main()
{
    /*
    如果设置gl_FragDepth = 1.f; 那么在fragment shader运行之前，
    这个fragment的depth是多少就不知道了，只能等fragment shader运行完才知道depth值，
    再进行depth test，early depth testing就没有用了
    */
    FragColor = vec4(vec3(texture(skybox, texture_coords)) * brightness, 1.f);
}