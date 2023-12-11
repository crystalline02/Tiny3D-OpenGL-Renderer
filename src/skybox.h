#pragma once
#include <glad/glad.h>

#include <string>
#include <vector>

class Camera;
namespace Shader
{
    class Sky_cube;
}

class Skybox
{
public:
    Skybox();
    void draw(const Shader::Sky_cube& shader, const Camera& camera, GLuint fbo = 0) const;
    inline void change_dir(unsigned int id) { m_cur_id = id; }
    inline std::vector<std::string> directories() const { return m_directories; }
    inline unsigned int selected() { return m_cur_id; }
    void set_selected(unsigned int index);
    inline float& intensity_ptr() { return m_intensity; }
    inline float intensity() const { return m_intensity; }
    inline GLuint cubmap_unit() const { return m_texture_unit; }
    static Skybox* get_instance();
    static void set_uniforms(const char* name, const Camera& camera, GLuint program);
private:
    inline std::vector<std::string> faces_vector(std::string base) const { 
        return { base + "/right.jpg", base + "/left.jpg", base + "/top.jpg", base + "/bottom.jpg", base + "/front.jpg", base + "/back.jpg" }; 
    }
private:
    GLuint VAO, VBO, m_texture_unit;
    float m_intensity;
    std::vector<std::string> m_directories;
    unsigned int m_cur_id;
    static Skybox* singleton;
};