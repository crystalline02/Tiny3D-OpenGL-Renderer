#version 460 core

in vec3 frag_pos;

uniform float near;
uniform float far;
uniform vec3 light_pos;

void main()
{
    vec3 dis_vec = light_pos - frag_pos;
    float dis = length(dis_vec);
    float max_dis = length(dis_vec * min(far / abs(dis_vec.x), min(far / abs(dis_vec.y), far / abs(dis_vec.z))));
    float min_dis = length(dis_vec * min(near / abs(dis_vec.x), min(near / abs(dis_vec.y), near / abs(dis_vec.z))));
    dis = (dis - min_dis) / (max_dis - min_dis);
    gl_FragDepth = dis;
}