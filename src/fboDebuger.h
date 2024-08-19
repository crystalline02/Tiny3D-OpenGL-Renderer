# pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

namespace Shader
{
    class FBODebuger;
}

class FBODebuger
{
public:
    static FBODebuger* getInstance(int width, int height);
    void draw(const Shader::FBODebuger& shader, GLuint texture_unit, bool alpha = false);
private:
    GLuint VAO, VBO, EBO;
    glm::mat4 m_model;
    int m_window_w, m_window_h;
    static FBODebuger* instance;
    FBODebuger(int width, int height);
};