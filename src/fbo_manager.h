#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "texture.h"


namespace FBO_Manager
{
    struct FBO_Data
    {
        GLuint fbo;
        GLuint rbo;
        std::vector<Texture> textures;

        static std::unordered_map<std::string, FBO_Data> windows_sized_fbos;
    };
    
    void create_scene_framebuffer_ms();
    void create_ssao_framebuffer();
    void create_pingpong_framebuffer_ms();
    void create_G_frambuffer();
    void gen_window_FBOs();
    void regen_window_FBOs();
    void clear_buffers();
}