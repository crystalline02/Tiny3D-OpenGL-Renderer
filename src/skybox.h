#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

class Camera;
namespace Shader
{
    class SkyCube;
    class HDRI2cubemap;
    class Cubemap2Irradiance;
    class CubemapPrefilter;
    class CubemapBRDFIntergral;
}
struct Texture;

class Skybox
{
public:
    void draw(const Shader::SkyCube& shader, const Camera& camera, GLuint fbo = 0) const;
    void draw_equirectangular_on_cubmap(const Shader::HDRI2cubemap& shader, const glm::mat4& view, 
        GLuint hdri_unit, GLuint fbo) const;
    void draw_irradiancemap(const Shader::Cubemap2Irradiance& shader, const glm::mat4& view, 
        GLuint cubemap_unit, GLuint fbo) const;
    void prefilt_cubemap(const Shader::CubemapPrefilter& shader, const glm::mat4& view, float roughness, 
        GLuint cubemap_unit, GLuint fbo, GLsizei width, GLsizei height) const;
    void BRDF_LUT_intergral(const Shader::CubemapBRDFIntergral& shader, GLuint fbo, GLsizei width, GLsizei height) const;
    inline void change_dir(unsigned int id) { m_cur_id = id; }
    inline std::vector<std::string> directories() const { return m_directories; }
    inline unsigned int selected() { return m_cur_id; }
    void set_selected(GLuint index);
    inline float& intensity_ptr() { return m_intensity; }
    inline float intensity() const { return m_intensity; }
    inline bool affect_scene() const { return m_affect_scene; }
    inline void set_affect_scene(bool affect) { m_affect_scene = affect; }
    GLuint cubemap_unit() const;
    GLuint irradiancemap_unit() const;
    GLuint prefilteredmap_unit() const;
    GLuint BRDF_LUT_unit() const;
    static Skybox* getInstance();
    static void set_uniforms(const char* name, const Camera& camera, GLuint program);
private:
    Skybox();
    inline std::vector<std::string> faces_vector(std::string base) const { 
        return { base + "/right.jpg", base + "/left.jpg", base + "/top.jpg", base + "/bottom.jpg", base + "/front.jpg", base + "/back.jpg" }; 
    }
private:
    GLuint VAO, VBO;
    Texture *m_cubemapTex, *m_tex_diffuseirrad, *m_tex_prefilter, *m_BRDF_LUT;
    bool m_affect_scene;
    float m_intensity;
    std::vector<std::string> m_directories;
    unsigned int m_cur_id;
    static Skybox* singleton;
};