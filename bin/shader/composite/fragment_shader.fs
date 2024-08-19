# version 460 core

// sceneMSFBO
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec2 textureCoord;

uniform sampler2D accum;
uniform sampler2D revealage;
uniform bool bloom;
uniform float threshold;

const float EPSILON = 1e-5;

bool approximatlyEqual(float a, float b);
float max3(vec3 v);
vec3 gamma_correct(vec3 color);

void main()
{
    ivec2 screenCoord = ivec2(gl_FragCoord.xy);
    
    vec4 accumColor = texelFetch(accum, screenCoord, 0);
    float revealage = texelFetch(revealage, screenCoord, 0).r;
    
    if(approximatlyEqual(revealage, 1.f)) discard;

    if(isinf(max3(accumColor.rgb))) accumColor.rgb = vec3(accumColor.a);

    accumColor.rgb /= max(accumColor.a, EPSILON);
    
    // blend Func: GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
    FragColor = vec4(accumColor.rgb, 1 - revealage);  // render target 'scene color attchment[0]'

    if(bloom)  // render target 'scene color attchment[1]'
    {
        float brightness = dot(accumColor.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
        if(brightness > threshold) BrightColor = vec4(accumColor.rgb, 1.f);
        else BrightColor = vec4(0.f, 0.f, 0.f, 1.f);
    }
    else BrightColor = vec4(0.f, 0.f, 0.f, 1.f);
}

vec3 gamma_correct(vec3 color)
{
    return pow(color, vec3(1.f / 2.2f));
}


bool approximatlyEqual(float a, float b)
{
    return abs(a - b) < max(abs(a), abs(b)) * EPSILON;
}

float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}