#include "fboDebuger.h"

#include "globals.h"
#include "shader.h"
#include "camera.h"

FBODebuger* FBODebuger::instance = nullptr;

FBODebuger::FBODebuger(int width, int height)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float vertex[] = {
        // location | texture coordinates
        -1.f, 1.f,    0.f, 1.f,
        1.f, 1.f,     1.f, 1.f,
        1.f, -1.f,    1.f, 0.f,
        -1.f, -1.f,   0.f, 0.f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    GLuint indices[] = {0, 1, 2, 0, 2, 3};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    for(GLuint i = 0; i < 2; ++i) glEnableVertexAttribArray(i);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int full_window_w = SRC_WIDTH;
    int full_window_h = util::Globals::camera.height();
    float ratio = float(height) / float(width);
    m_window_w = glm::sqrt(float(full_window_w * full_window_h) / float(8.f * ratio));
    m_window_h = m_window_w * ratio;

    // define model matrix
    m_model = glm::mat4(1.f);
    // 先制定好矩阵表达的位移属性
    m_model = glm::translate(m_model, {float(full_window_w - m_window_w) / float(full_window_w), 
        float(full_window_h - m_window_h) / float(full_window_h), 
        0.f});
    // 然后再制定好矩阵表达的旋转/缩放属性
    m_model = glm::scale(m_model, {float(m_window_w) / float(full_window_w), 
        float(m_window_h) / float(full_window_h), 
        1.f});
}

FBODebuger* FBODebuger::getInstance(int width, int height)
{
    return instance ? instance : (instance = new FBODebuger(width, height));
}

void FBODebuger::draw(const Shader::FBODebuger& shader, GLuint textureUnit, bool alpha)
{
    assert(shader.shaderName() == "./shader/FBODebuger");
    int camera_width = SRC_WIDTH;
    int camera_height = util::Globals::camera.height();
    glBindVertexArray(VAO);
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    for(GLuint i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.set_uniforms(textureUnit, m_model, alpha);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    
    glBindVertexArray(0);
}