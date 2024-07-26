# pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <string>

#include "texture.h"

namespace Shader
{
    class TextShader;
}

struct Character
{
    Texture texture;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class CharacterRender
{
public:
    void render_text(const Shader::TextShader& shader, std::string texts, float x, float y, float scale, 
        const glm::vec3& color) const;
    void set_projection(int window_w, int window_h);
    void init_fonts(const char* font);
    
    static CharacterRender* getInstance();
private:
    CharacterRender();
    glm::mat4 m_projection;
    GLuint VAO, VBO;
    std::unordered_map<char, Character> m_characters;

    static CharacterRender* instance;
};