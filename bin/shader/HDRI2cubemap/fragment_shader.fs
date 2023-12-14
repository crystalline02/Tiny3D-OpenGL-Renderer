# version 460 core

uniform sampler2D equirectangularmap;

in vec3 frag_pos;  // world

out vec4 FragColor;

vec2 sample_sphericalmap(vec3 direction)
{
    vec2 uv = vec2(atan(direction.z, direction.x), asin((direction.y)));
    uv *= vec2(0.1591, 0.3183);
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = sample_sphericalmap(normalize(frag_pos));
    FragColor = vec4(texture(equirectangularmap, uv).rgb, 1.f);
}