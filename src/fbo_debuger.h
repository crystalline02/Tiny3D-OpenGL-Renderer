# pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

namespace Shader
{
    class FBO_debuger;
}

class FBO_debuger
{
public:
    static FBO_debuger* get_instance(int width, int height);
    void draw(const Shader::FBO_debuger& shader, GLuint texture_unit);
private:
    GLuint VAO, VBO, EBO;
    glm::mat4 m_model;
    int m_window_w, m_window_h;
    static FBO_debuger* instance;
    FBO_debuger(int width, int height);
};