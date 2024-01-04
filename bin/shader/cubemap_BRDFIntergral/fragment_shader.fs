# version 460 core

in vec2 tex_coord;

out vec2 FragColor;

const uint sample_count = 4096u;
const float PI = 3.14159265359;


vec2 BRDF_intergral(float NdotV, float roughness);
vec2 Hammersley_points(uint i, uint N);
vec3 importance_sampling_GGX(vec2 pt, float roughness, vec3 N);
float GeometrySchlickGGX(vec3 n, vec3 v, float roughness);
float GeometrySmith(vec3 n, vec3 v, vec3 l, float roughness);
float Radical_Inverse_vdc(uint bits);

// computing BRDF part of specular IBL, that is \int_\Omega f_r(p, w_i, w_o) n \cdot w_i\,dw_i
void main()
{
    vec2 scale_bias = BRDF_intergral(tex_coord.x, tex_coord.y);
    FragColor = scale_bias;
}

// for each NdotV and roughness pair, store a precomputed value
vec2 BRDF_intergral(float NdotV, float roughness)
{
    // precomputing in tangent space
    vec3 N = vec3(0.f, 0.f, 1.f);
    vec3 V = vec3(sqrt(1.f - NdotV * NdotV), 0.f, NdotV);

    float A = 0.f;
    float B = 0.f;
    for(uint i = 0; i < sample_count; ++i)
    {
        vec3 H = importance_sampling_GGX(Hammersley_points(i, sample_count), roughness, N);
        vec3 L = reflect(-V, H);
        float VdotH = max(dot(V, H), 0.f);
        float NdotH = max(H.z, 0.f);
        float NdotL = max(L.z, 0.f);

        // 这里必须有NdotL > 0.f的条件判定，应为当L超出半球范围内时，L不会对这个\int_\Omega f_r(p, w_i, w_o) n \cdot w_i\, dw_i积分有贡献
        if(NdotL > 0.f)
        {
            float alpha = pow(1.f - VdotH, 5.f);
            float G_vis = (GeometrySmith(N, V, L, roughness) * VdotH) / (NdotV * NdotH);

            A += G_vis * (1.f - alpha);
            B += G_vis * alpha;
        }
    }
    A /= float(sample_count);
    B /= float(sample_count);

    return vec2(A, B);
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

float GeometrySchlickGGX(vec3 n, vec3 v, float roughness)
{
    // float k = (roughness + 1) * (roughness + 1) / 8;
    float a = roughness;
    float k = (a * a) / 2.f;
    float n_dot_v = max(dot(n, v), 0);
    float numerator = n_dot_v;
    float denominator = n_dot_v * (1 - k) + k;
    return numerator / denominator;
}

float GeometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
    return GeometrySchlickGGX(n, v, roughness) * GeometrySchlickGGX(n, l, roughness);
}