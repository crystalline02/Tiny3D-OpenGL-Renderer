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
float DistributionGGX(vec3 n, vec3 h, float roughness);

// cacualating the prefiltered part of specular IBL, that is \int_\Omega L(p, w_i)\,dw_i
// for each R and roughness, store a precomputed value
void main()
{
    /*
    这里frag_pos代表R，意思是，如果我想获得在某一个V和N下的积分\int_\Omega L(p, w_i)\, dw_i，那么只需用
    R = relfect(-V, N)去采样这个prefilter enviroment map，这个cubemap提供了在这个特定的R下的积分结果。
    */
    vec3 R = normalize(frag_pos);
    vec3 N = R;
    vec3 V = N;

    vec3 result = vec3(0.f);
    float total_weight = 0.f;
    for(uint i = 0; i < sample_count; ++i)
    {
        vec2 pt = Hammersley_points(i, sample_count);
        vec3 H = importance_sampling_GGX(pt, roughness, N);
        vec3 L = normalize(-V + 2 * H * max(dot(V, H), 0.f));

        float NdotL = max(dot(N, L), 0.f);
        float NdotH = max(dot(N, H), 0.f);
        float HdotL = max(dot(H, L), 0.f);
        
        float D = DistributionGGX(N, H, roughness);
        float pdf = D * NdotH / (4 * HdotL) + 0.000001f;   // 这个pdf可以证明的，见《Notes on Ward BRDF》和上面我的笔记
        float sa_texel = 4 * PI / (6 * 2048 * 2048);  // 每个texel占多大的立体角
        float sa_sample = 1.f / (float(sample_count) * pdf + 0.000001f);  // 每个sample的L占多大的立体角
        float mip_level = roughness == 0.f ? 0.f : 0.5 * log2(sa_sample / sa_texel);  // 0.5 * log2(sa_sample / sa_texel)为准确的要选取的mip level

        // if(NdotL > 0)  // 如果计算得到的采样向量L与N成90度，可以不累加（实际上这里写不写这个判断条件都可以）

        result += texture(enviroment_map, L, mip_level).rgb * NdotL;  // 为什么这里的权重为NdotL，公式推导也可以见知乎文章和上面我的笔记
        total_weight += NdotL;
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

float DistributionGGX(vec3 n, vec3 h, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float n_dot_h = max(dot(n, h), 0.f);
    float n_dot_h2 = n_dot_h * n_dot_h;
    float denominator = n_dot_h2 * (a2 - 1) + 1;
    denominator = denominator * denominator * PI;
    return a2 / denominator;
}