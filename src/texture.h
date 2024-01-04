#pragma once
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct Texture
{
    Texture(std::string name = "Null texture", GLuint texuint = 0, GLuint texbuffer = 0): 
        texname(name), texunit(texuint), texbuffer(texbuffer) { }
    std::string texname;
    GLuint texunit;
    GLuint texbuffer;
};
