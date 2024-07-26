# version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTextureCoord;

uniform mat4 model;
uniform mat3 normalMat;

out VSOut
{
    vec3 normal;
    vec3 tangent;
    vec2 textureCoord;
} vsOut;

void main()
{
    gl_Position = model * vec4(aPosition, 1.f);  // world
    vsOut.normal = normalize(normalMat * aNormal);  // world
    vsOut.tangent = normalize(aTangent * aTangent);  // world
    vsOut.textureCoord = aTextureCoord;
}