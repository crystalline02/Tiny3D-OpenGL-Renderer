#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "texture.h"


namespace FBOManager
{
    struct FBOData
    {
        GLuint fbo;
        GLuint rbo;
        std::vector<Texture> textures;

        static std::unordered_map<std::string, FBOData> windows_sized_fbos;
    };
    
    void createTransparentFBO();
    void createSceneFramebufferMS();
    void createSSAOFramebuffer();
    void createPingpongFramebufferMS();
    void createGFrambuffer();
    void genWindowFBOs();
    void regenWindowFBOs();
    void clearBuffers();
}