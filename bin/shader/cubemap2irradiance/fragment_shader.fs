# version 460 core

const float PI = 3.14159265359;

uniform samplerCube env_map;

out vec4 FragColor;

in vec3 frag_pos;

void main()
{
    /* 这里N代表一个法线向量，意思是，如果我们要获得一个fragment在这个envriment map下的diffuse irradiance，那么就
    要用这个fragment的法线去采用irradiance map，这个irradiance map提供了在这个法线下，这个fragment的
    diffuse irradiance，并且这个值在所有的wo方向下都一致。
    */
    vec3 N = normalize(frag_pos);

    vec3 up = abs(N.z) < 0.9999f ? vec3(0.f, 1.f, 0.f) : vec3(0.f, 0.f, 1.f);  // 这一步也是很重要的，每次对两个向量做叉积，都要避免两个向量是平行的，虽然这种情况很难出现
    vec3 normal_z = N;  // 由于我们在tangent space下用vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));表示向量，所以这里是把法线方向当z轴了
    vec3 right_x = normalize(cross(up, normal_z));
    vec3 up_y = normalize(cross(normal_z, right_x));

    mat3 TBN = mat3(right_x, up_y, normal_z);  // tangent->world
    
    float sample_delta = 0.008f;
    vec3 irradiance = vec3(0.f);
    int n_samples = 0;
    for(float phi = 0; phi <= 2.0f * PI; phi += sample_delta)
    {
        for(float theta = 0; theta <= PI * 0.5f; theta += sample_delta)
        {
            vec3 sample_tangent = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sample_world = TBN * sample_tangent;
            irradiance += texture(env_map, sample_world).rgb * cos(theta) * sin(theta);
            ++n_samples;
        }
    }
    irradiance *= (PI / float(n_samples));

    FragColor = vec4(irradiance, 1.f);
}