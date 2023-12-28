# pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <string>


struct Texture;
namespace Shader
{
    class Text_shader;
}

struct Character
{
    Texture* texture;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class Character_Render
{
public:
    Character_Render(const char* font);
    void render_text(const Shader::Text_shader& shader, std::string texts, float x, float y, float scale, 
        const glm::vec3& color) const;
private:
    glm::mat4 m_projection;
    GLuint VAO, VBO;
    std::unordered_map<char, Character> m_characters;
    void init_fonts(const char* font);
};