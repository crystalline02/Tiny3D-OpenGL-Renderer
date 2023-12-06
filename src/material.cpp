#include "material.h"
#include "globals.h"
#include <string>

Material::Material(const glm::vec3& diffuse_color_, 
    const glm::vec3& specular_color_, 
    const glm::vec3& ambient_color_,
    float opacity):
m_diffuse_color(diffuse_color_), m_specular_color(specular_color_), m_ambient_color(ambient_color_), 
m_opacity(opacity), m_diffuse_map_units(), m_specular_map_units(), m_ambient_map_units(), 
m_opacity_map_units(), m_metalic_map_units(), m_roughness_map_units(), m_shinness(32), m_roughness(0.5f),
m_normal_stength(1.f), m_type(Mat_type::Blinn_Phong)
{
    
}

Material::Material(const std::vector<unsigned int>& diffuse_map_units, 
    const std::vector<unsigned int>& specular_map_units, 
    const std::vector<unsigned int>& roughness_map_units,
    const std::vector<unsigned int>& ambient_map_units, 
    const std::vector<unsigned int>& opacity_map_units,
    const std::vector<unsigned int>& normal_map_units,
    const std::vector<unsigned int>& displacement_map_units,
    const std::vector<unsigned int>& metalic_map_units):
m_diffuse_map_units(diffuse_map_units), 
m_specular_map_units(specular_map_units), 
m_roughness_map_units(roughness_map_units),
m_ambient_map_units(ambient_map_units), 
m_opacity_map_units(opacity_map_units),
m_normal_map_units(normal_map_units),
m_displacement_map_units(displacement_map_units),
m_metalic_map_units(metalic_map_units),
m_diffuse_color(glm::vec3(1.f)), m_specular_color(glm::vec3(.5f)), m_ambient_color(glm::vec3(0.1f)), 
m_opacity(1.f), m_shinness(32), m_normal_stength(1.f), m_type(Mat_type::Blinn_Phong)
{

}

void Material::set_uniforms(const char* name, GLint program) const
{
    std::string albedo = std::string(name) + std::string(".albedo_color");
    std::string opacity = std::string(name) + std::string(".opacity");
    std::string metalic = std::string(name) + std::string(".metalic");
    std::string normal_map_strength = std::string(name) + std::string(".normal_map_strength");
    std::string use_albedo_map = std::string(name) + std::string(".use_albedo_map");
    std::string use_opacity_map = std::string(name) + std::string(".use_opacity_map");
    std::string use_roughness_map = std::string(name) + std::string(".use_roughness_map");
    std::string use_normal_map = std::string(name) + std::string(".use_normal_map");
    std::string use_displacement_map = std::string(name) + std::string(".use_displacement_map");
    std::string use_metalic_map = std::string(name) + std::string(".use_metalic_map");
    std::string diffuse_color = (std::string(name) + std::string(".diffuse_color"));
    std::string specular_color = (std::string(name) + std::string(".specular_color"));
    std::string ambient_color = (std::string(name) + std::string(".ambient_color"));
    std::string use_diffuse_map = (std::string(name) + std::string(".use_diffuse_map"));
    std::string use_specular_map = (std::string(name) + std::string(".use_specular_map"));
    std::string use_ambient_map = (std::string(name) + std::string(".use_ambient_map"));
    std::string shinness = (std::string(name) + std::string(".shinness"));
    std::string roughness = (std::string(name) + std::string(".roughness"));

    switch(m_type)
    {
        case Mat_type::Blinn_Phong:
        {
            // diffuse
            if(!m_diffuse_map_units.empty()) 
            {
                for(int i = 0; i < m_diffuse_map_units.size(); ++i) 
                {
                    std::string diffuse_map = (std::string(name) + std::string(".diffuse_maps[") + std::to_string(i) + std::string("]"));
                    util::set_int(diffuse_map.c_str(), m_diffuse_map_units[i], program);
                }
                util::set_bool(use_diffuse_map.c_str(), GL_TRUE, program);
            }
            else 
            {
                util::set_floats(diffuse_color.c_str(), m_diffuse_color, program);
                util::set_bool(use_diffuse_map.c_str(), GL_FALSE, program);
            }
            
            // specular
            if(!m_specular_map_units.empty()) 
            {
                for(int i = 0; i < m_specular_map_units.size(); ++i)
                {
                    std::string specular_map = (std::string(name) + std::string(".specular_maps[") + std::to_string(i) + std::string("]"));
                    util::set_int(specular_map.c_str(), m_specular_map_units[i], program);
                }
                util::set_bool(use_specular_map.c_str(), GL_TRUE, program);
            }
            else 
            {
                util::set_floats(specular_color.c_str(), m_specular_color, program);
                util::set_int(use_specular_map.c_str(), GL_FALSE, program);
            }
            
            // ambient
            if(!m_ambient_map_units.empty()) 
            {
                for(int i = 0; i < m_ambient_map_units.size(); ++i)
                {
                    std::string ambient_map = (std::string(name) + std::string(".ambient_maps[") + std::to_string(i) + std::string("]"));
                    util::set_int(ambient_map.c_str(), m_ambient_map_units[i], program);
                }
                util::set_bool(use_ambient_map.c_str(), GL_TRUE, program);
            }
            else
            {
                util::set_floats(ambient_color.c_str(), m_ambient_color, program);
                util::set_bool(use_ambient_map.c_str(), GL_FALSE, program);
            }
            
            // opacity
            if(!m_opacity_map_units.empty())
            {
                for(int i = 0; i < m_opacity_map_units.size(); ++i)
                {
                    std::string opacity_map = (std::string(name) + std::string(".opacity_maps[") + std::to_string(i) + std::string("]"));
                    util::set_int(opacity_map.c_str(), m_opacity_map_units[i], program);
                }
                util::set_bool(use_opacity_map.c_str(), GL_TRUE, program);
            }
            else
            {
                util::set_float(opacity.c_str(), m_opacity, program);
                util::set_bool(use_opacity_map.c_str(), GL_FALSE, program);
            }
            
            // normal map
            if(!m_normal_map_units.empty())
            {
                for(int i = 0; i < m_normal_map_units.size(); ++i)
                {
                    std::string normal_map = std::string(name) + ".normal_maps[" + std::to_string(i) + "]";
                    util::set_int(normal_map.c_str(), m_normal_map_units[i], program);
                }
                util::set_bool(use_normal_map.c_str(), GL_TRUE, program);
                util::set_float(normal_map_strength.c_str(), m_normal_stength, program);
            }
            else
                util::set_bool(use_normal_map.c_str(), GL_FALSE, program);

            // dispacement map
            if(!m_displacement_map_units.empty())
            {
                for(int i = 0; i < m_displacement_map_units.size(); ++i)
                {
                    std::string displacement_map = std::string(name) + ".displacement_maps[" + std::to_string(i) + "]";
                    util::set_int(displacement_map.c_str(), m_displacement_map_units[i], program);
                }
                util::set_bool(use_displacement_map.c_str(), GL_TRUE, program);
            }
            else
                util::set_bool(use_displacement_map.c_str(), GL_FALSE, program);

            // shinness
            util::set_float(shinness.c_str(), m_shinness, program);
            break;
        }
        case Mat_type::PBR:
        {
            // albedo
            if(!m_diffuse_map_units.empty())
            {
                for(int i = 0; i < m_diffuse_map_units.size(); ++i)
                {
                    std::string albedo_maps = std::string(name) + std::string(".albedo_maps[") + std::to_string(i) + std::string("]");
                    util::set_int(albedo_maps.c_str(), m_diffuse_map_units[i], program);
                }
                util::set_bool(use_albedo_map.c_str(), true, program);
            }
            else
            {
                util::set_bool(use_albedo_map.c_str(), false, program);
                util::set_floats(albedo.c_str(), m_diffuse_color, program);
            }
            
            // opacity
            if(!m_opacity_map_units.empty())
            {
                for(int i = 0; i < m_opacity_map_units[i]; ++i)
                {
                    std::string opacity_maps = std::string(name) + std::string(".opacity_maps[") + std::to_string(i) + std::string("]");
                    util::set_int(opacity_maps.c_str(), m_opacity_map_units[i], program);
                }
                util::set_bool(use_opacity_map.c_str(), true, program);
            }
            else
            {
                util::set_bool(use_opacity_map.c_str(), false, program);
                util::set_float(opacity.c_str(), m_opacity, program);
            }
            
            // roughness
            if(!m_roughness_map_units.empty())
            {
                for(int i = 0; i < m_roughness_map_units.size(); ++i)
                {
                    std::string roughness_maps = std::string(name) + std::string(".roughness_maps[") + std::to_string(i) + std::string("]");
                    util::set_int(roughness_maps.c_str(), m_roughness_map_units[i], program);
                }
                util::set_bool(use_roughness_map.c_str(), true, program);
            }
            else
            {
                util::set_bool(use_roughness_map.c_str(), false, program);
                util::set_float(roughness.c_str(), m_roughness, program);
            }

            // normal
            if(!m_normal_map_units.empty())
            {
                for(int i = 0; i < m_normal_map_units.size(); ++i)
                {
                    std::string normal_maps = std::string(name) + std::string(".normal_maps[") + std::to_string(i) + std::string("]");
                    util::set_int(normal_maps.c_str(), m_normal_map_units[i], program);
                }
                util::set_bool(use_normal_map.c_str(), true, program);
                util::set_float(normal_map_strength.c_str(), m_normal_stength, program);
            }
            else
                util::set_bool(use_normal_map.c_str(), false, program);

            // displacement
            if(!m_displacement_map_units.empty())
            {
                for(int i = 0; i < m_displacement_map_units.size(); ++i)
                {
                    std::string displacement_maps = std::string(name) + std::string(".displacement_maps[") + std::to_string(i) + std::string("]");
                    util::set_int(displacement_maps.c_str(), m_displacement_map_units[i], program);
                }
                util::set_bool(use_displacement_map.c_str(), true, program);
            }
            else
                util::set_bool(use_displacement_map.c_str(), false, program);

            // metalic
            if(!m_metalic_map_units.empty())
            {
                for(int i = 0; i < m_metalic_map_units.size(); ++i)
                {
                    std::string metalic_maps = std::string(name) + std::string(".metalic_maps[") + std::to_string(i) + std::string("]");
                    util::set_int(metalic_maps.c_str(), m_metalic_map_units[i], program);
                }
                util::set_bool(use_metalic_map.c_str(), true, program);
            }
            else
            {
                util::set_bool(use_metalic_map.c_str(), false, program);
                util::set_float(metalic.c_str(), m_metalic, program);
            }

            // ambient
            if(!m_ambient_map_units.empty()) 
            {
                for(int i = 0; i < m_ambient_map_units.size(); ++i)
                {
                    std::string ambient_map = (std::string(name) + std::string(".ambient_maps[") + std::to_string(i) + std::string("]"));
                    util::set_int(ambient_map.c_str(), m_ambient_map_units[i], program);
                }
                util::set_bool(use_ambient_map.c_str(), GL_TRUE, program);
            }
            else
            {
                util::set_floats(ambient_color.c_str(), m_ambient_color, program);
                util::set_bool(use_ambient_map.c_str(), GL_FALSE, program);
            }
            break;
        }
        default:
            break;
    }
}

void Material::switch_mat_type(Mat_type new_type)
{
    m_type = new_type;
}
