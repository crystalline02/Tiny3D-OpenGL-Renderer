# version 460 core

// historyFBO
layout(location = 0) out vec4 FragColor;

in vec2 textureCoord;

uniform sampler2D currentColorTex;
uniform sampler2D historyColorTex;

const float factor = 0.9;

void main()
{
    vec4 currentColor = texture(currentColorTex, textureCoord);

    vec4 historyColor = texture(historyColorTex, textureCoord);

    FragColor = vec4(mix(currentColor.rgb, historyColor.rgb, factor), 1.f);

}