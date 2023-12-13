#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <string>
#include <vector>

struct Texture
{
    Texture(std::string name = "Null texture", GLuint texuint = 0, GLuint texbuffer = 0): 
        m_name(name), m_texunit(texuint), m_texbuffer(texbuffer) { }
    std::string m_name;
    GLuint m_texunit;
    GLuint m_texbuffer;
};

enum class Mat_type
{
    Blinn_Phong, 
    PBR
};

class Material
{
public:
    Material(const glm::vec3& diffuse_color = glm::vec3(1.f), 
        const glm::vec3& specular_color = glm::vec3(1.f), 
        const glm::vec3& ambient_color = glm::vec3(1.f),
        float opacity = 1.f,
        Mat_type mat_type = Mat_type::Blinn_Phong);
    Material(const std::vector<Texture>& diffuse_textures,
        const std::vector<Texture>& specular_textures,
        const std::vector<Texture>& roughness_textures, 
        const std::vector<Texture>& ambient_textures,
        const std::vector<Texture>& opacity_textures,
        const std::vector<Texture>& normal_textures,
        const std::vector<Texture>& displacement_textures,
        const std::vector<Texture>& metalic_textures,
        Mat_type mat_type = Mat_type::Blinn_Phong);
    ~Material();
    void set_uniforms(const char* name, GLint program) const;
    void switch_mat_type(Mat_type new_type);
    inline void set_diffuse_color(const glm::vec3& color) { m_diffuse_color = color; }
    inline void set_specular_color(const glm::vec3& color) { m_specular_color = color; }
    inline void set_ambient_color(const glm::vec3& color) { m_ambient_color  = color; }
    inline void set_metalic(const float& metalic) { m_metalic = metalic; }
    inline void set_shinness(float shinness) { m_shinness = shinness; }
    inline void set_roughness(float roughness) {m_roughness = roughness;}
    inline int shinness() { return m_shinness; }
    inline void set_normal_strength(float strength) { m_normal_stength = strength; }
    inline float normal_strength() { return m_normal_stength; }
    inline void set_opacity(float opacity) { m_opacity = opacity; }
    inline bool is_blend_mat() const { return (!m_opacity_textures.empty()) || (m_opacity_textures.empty() && m_opacity < 1.f); }
    inline Mat_type mat_type() const { return m_type; }
private:
    glm::vec3 m_diffuse_color, m_specular_color, m_ambient_color;
    std::vector<Texture> m_diffuse_textures, 
        m_specular_textures, 
        m_ambient_textures, 
        m_opacity_textures,
        m_normal_textures,
        m_displacement_textures,
        m_metalic_textures,
        m_roughness_textures;
    int m_shinness;
    Mat_type m_type;
    float m_opacity, m_normal_stength, m_metalic, m_roughness;
};
