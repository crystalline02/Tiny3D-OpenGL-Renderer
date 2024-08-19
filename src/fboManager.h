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

        static std::unordered_map<std::string, FBOData> windowsSizeFBOs;
    };
    
    void createTransparentFBO();
    void createSceneFramebufferMS();
    void createSceneFramebuffer();
    void createSSAOFramebuffer();
    void createPingpongFramebuffer();
    void createPingpongFramebufferMS();
    void createGFrambuffer();
    void createHistoryFramebuffer();
    void genWindowFBOs();
    void regenWindowFBOs();
    void clearBuffers();
}