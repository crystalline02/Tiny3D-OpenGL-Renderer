#include "skybox.h"
#include "shader.h"
#include "camera.h"
#include "globals.h"
#include "model.h"
#include "material.h"

#include <filesystem>


Skybox::Skybox(): m_intensity(1.f), m_cur_id(2)
{
    float cube_vertices[] = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f
    };
    glGenVertexArrays(1, &VAO);
    VBO = util::create_vbo_2_vao(cube_vertices, sizeof(cube_vertices));
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Load skybox and HDRIs from directory
    for(const std::filesystem::directory_entry& entry: std::filesystem::directory_iterator("./cubemap"))
    {
        if(entry.is_directory()) 
        {
            if(entry.path().filename() != "HDRIs") m_directories.push_back(entry.path().string());
            else
            {
                for(const std::filesystem::directory_entry& entry_HDRIs: std::filesystem::directory_iterator(entry.path()))
                {
                    if(entry_HDRIs.is_regular_file() && entry_HDRIs.path().extension() == ".hdr")
                        m_directories.push_back(entry_HDRIs.path().string());
                }
            }
        }
    }

    if(std::filesystem::is_directory(m_directories[m_cur_id]))
        util::create_cubemap(faces_vector(m_directories[m_cur_id]), *m_texture);
}

void Skybox::set_selected(GLuint index)
{
    m_cur_id = index;
    Model::rm_texture(*m_texture);
    util::create_cubemap(faces_vector(m_directories[m_cur_id]), *m_texture);
}

Skybox* Skybox::get_instance()
{
    if(singleton == nullptr) return singleton = new Skybox();
    else return singleton;
}

GLuint Skybox::cubmap_unit() const
{
    return m_texture->m_texunit;
}

void Skybox::draw(const Shader::Sky_cube& shader, const Camera& camera, GLuint fbo) const
{
    assert(shader.shader_dir() == "./shader/sky_cube");
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(VAO);
    shader.use();
    shader.set_uniforms(camera, m_intensity, m_texture->m_texunit);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void Skybox::set_uniforms(const char* name, const Camera& camera, GLuint program)
{
    std::string cubemap = std::string(name) + ".cubemap";
    std::string intensity = std::string(name) + ".intensity";
    std::string use = std::string(name) + ".use";
    if(util::Globals::skybox)
    {
        Skybox* skybox = Skybox::get_instance();
        util::set_int(cubemap.c_str(), skybox->cubmap_unit(), program);
        util::set_float(intensity.c_str(), skybox->intensity(), program);
        util::set_bool(use.c_str(), true, program);
    }
    else util::set_bool(use.c_str(), false, program);
}

Skybox* Skybox::singleton = nullptr;