# version 460 core

in vec3 frag_pos;
out vec4 FragColor;

uniform float roughness;
uniform samplerCube enviroment_map;

const float PI = 3.14159265359;
const uint sample_count = 4096u;


float Radical_Inverse_vdc(uint bits);
vec2 Hammersley_points(uint i, uint N);
vec3 importance_sampling_GGX(vec2 pt, float roughness, vec3 N);

// cacualating the prefiltered part of specular IBL, that is \int_\Omega L(p, w_i)\,dw_i
void main()
{
    vec3 N = normalize(frag_pos);
    vec3 V = N;
    vec3 result = vec3(0.f);
    float total_weight = 0.f;
    for(uint i = 0; i < sample_count; ++i)
    {
        vec2 pt = Hammersley_points(i, sample_count);
        vec3 H = importance_sampling_GGX(pt, roughness, N);
        vec3 L = normalize(-V + 2 * H * max(dot(V, H), 0.f));

        float NdotL = max(dot(N, L), 0.f);
        result += texture(enviroment_map, L).rgb * NdotL;
        total_weight += NdotL;  // 当L和N的夹角小，则我们认为这根光线对V方向的specular贡献大（我也不知道为什么这样）
    }
    result /= total_weight;
    FragColor = vec4(result, 1.f);
}

float Radical_Inverse_vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);  // 交换bits的前16位和后16位
    bits = ((bits & 0x55555555) << 1u) | ((bits & 0xAAAAAAAA) >> 1u);  // 交换bits中每2位中的前1位和后1位
    bits = ((bits & 0x33333333) << 2u) | ((bits & 0xCCCCCCCC) >> 2u);  // 交换bits中每4位中的前2位和后2位
    bits = ((bits & 0xF0F0F0F0) << 4u) | ((bits & 0x0F0F0F0F) >> 4u);  // 交换bits中每8位中的前4位和后4位
    bits = ((bits & 0x00FF00FF) << 8u) | ((bits & 0xFF00FF00) >> 8u);  // 交换bits中每8位中的前4位和后4位
    return float(bits) * 2.3283064365386963e-10;  // 这里就相当于除以2^32，把bits向右移动32位，至此，实现了二进制位按照小数点镜像
}

vec2 Hammersley_points(uint i, uint N)
{
    return vec2(float(i) * (1.f / float(N)), Radical_Inverse_vdc(i));
}

vec3 importance_sampling_GGX(vec2 pt, float roughness, vec3 N)
{
    float a = roughness * roughness;
    float phi = 2.0f * pt.x * PI;
    float cosTheta = sqrt((1.f - pt.y) / (1.f + (a * a - 1.f) * pt.y));
    float sinTheta = sqrt(1 - cosTheta * cosTheta);

    vec3 H_tangent = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
    
    vec3 normal_z = N;
    vec3 up = abs(N.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(1.f, 0.f, 0.f);  // 这一步是为了避免接下来计算right_x这一步时（计算N和up叉积），平行向量叉积结果为0
    vec3 right_x = normalize(cross(up, normal_z));
    vec3 up_y = normalize(cross(normal_z, right_x));

    mat3 TBN = mat3(right_x, up_y, normal_z);
    
    vec3 H_world = normalize(TBN * H_tangent);
    return H_world;
}