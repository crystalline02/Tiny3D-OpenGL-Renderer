#include "skybox.h"
#include "shader.h"
#include "camera.h"
#include "globals.h"
#include "model.h"

#include <filesystem>


Skybox::Skybox(): m_intensity(1.f), m_cur_id(0)
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
    
    for(const std::filesystem::directory_entry& entry: std::filesystem::directory_iterator("./cubemap"))
        if(std::filesystem::is_directory(entry.path())) m_directories.push_back(entry.path().string());
    m_texture_unit = Model::num_textures();
    util::create_cubemap(m_texture_unit, faces_vector(m_directories[m_cur_id]));
}

void Skybox::set_selected(unsigned int index)
{
    m_cur_id = index;
    util::create_cubemap(m_texture_unit, faces_vector(m_directories[m_cur_id]));
}

Skybox* Skybox::get_instance()
{
    if(singleton == nullptr) return singleton = new Skybox();
    else return singleton;
}

void Skybox::draw(const Shader::Sky_cube& shader, const Camera& camera, GLuint fbo) const
{
    assert(shader.shader_dir() == "./shader/sky_cube");
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(VAO);
    shader.use();
    shader.set_uniforms(camera, m_intensity, m_texture_unit);
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