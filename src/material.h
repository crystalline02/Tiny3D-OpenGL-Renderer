#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

class Material
{
public:
    Material(const glm::vec3& diffuse_color = glm::vec3(1.f), 
        const glm::vec3& specular_color = glm::vec3(1.f), 
        const glm::vec3& ambient_color = glm::vec3(1.f),
        float opacity = 1.f);
    Material(const std::vector<unsigned int>& diffuse_map_units, 
        const std::vector<unsigned int>& specular_map_units, 
        const std::vector<unsigned int>& ambient_map_units,
        const std::vector<unsigned int>& opacity_map_units,
        const std::vector<unsigned int>& normal_map_units,
        const std::vector<unsigned int>& displacement_map_units);
    void set_uniforms(const char* name, GLint program) const;
    inline void set_diffuse_color(const glm::vec3& color) { m_diffuse_color = color; }
    inline void set_specular_color(const glm::vec3& color) { m_specular_color = color; }
    inline void set_ambient_color(const glm::vec3& color) { m_ambient_color  = color; }
    inline void set_shinness(float shinness) { m_shinness = shinness; }
    inline int shinness() { return m_shinness; }
    inline void set_normal_strength(float strength) { m_normal_stength = strength; }
    inline float normal_strength() { return m_normal_stength; }
    inline void set_opacity(float opacity) { m_opacity = opacity; }
    inline bool is_blend_mat() const { return (!m_opacity_map_units.empty()) || (m_opacity_map_units.empty() && m_opacity < 1.f); }
private:
    glm::vec3 m_diffuse_color, m_specular_color, m_ambient_color;
    std::vector<unsigned int> m_diffuse_map_units, 
        m_specular_map_units, 
        m_ambient_map_units, 
        m_opacity_map_units,
        m_normal_map_units,
        m_displacement_map_units;
    int m_shinness;
    float m_opacity, m_normal_stength;
};

