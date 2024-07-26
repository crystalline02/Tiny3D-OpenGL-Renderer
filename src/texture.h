#pragma once
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct Texture
{
    Texture(std::string name = "Null texture", GLuint texuint = 0, GLuint texbuffer = 0): 
        texName(name), texUnit(texuint), texBuffer(texbuffer) { }
    std::string texName;
    GLuint texUnit;
    GLuint texBuffer;
};
