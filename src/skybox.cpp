#include "skybox.h"
#include "shader.h"
#include "camera.h"
#include "globals.h"
#include "model.h"
#include "material.h"

#include <filesystem>


Skybox::Skybox(): m_intensity(1.f), m_cur_id(3), m_texture(new Texture()), m_affect_scene(false)
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
    else
        util::create_cubemap(m_directories[m_cur_id].c_str(), *m_texture);
}

void Skybox::set_selected(GLuint index)
{
    m_cur_id = index;
    Model::rm_texture(*m_texture);
    if(std::filesystem::is_directory(m_directories[m_cur_id]))
        util::create_cubemap(faces_vector(m_directories[m_cur_id]), *m_texture);
    else
        util::create_cubemap(m_directories[m_cur_id].c_str(), *m_texture);
    
    // std::cout << m_texture->m_name << ", " << m_texture->m_texunit << ", " << m_texture->m_texbuffer << std::endl;
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
    assert(shader.shader_name() == "./shader/sky_cube");
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

void Skybox::draw_equirectangular_on_cubmap(const Shader::HDRI2cubemap& shader, const glm::mat4& view, GLuint hdri_unit, GLuint fbo) const
{
    assert(shader.shader_name() == "./shader/HDRI2cubemap");
    glViewport(0, 0, 1024, 1024);  // Important step!
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Draw once only when initializing skybox or swithing skybox, NOT each frame.
    
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    shader.use();
    shader.set_uniforms(hdri_unit, view);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, util::Globals::camera.width(), util::Globals::camera.height());
}

void Skybox::set_uniforms(const char* name, const Camera& camera, GLuint program)
{
    std::string cubemap = std::string(name) + ".cubemap";
    std::string intensity = std::string(name) + ".intensity";
    std::string use = std::string(name) + ".use";
    std::string affect_scene = std::string(name) + ".affect_scene";
    if(util::Globals::skybox)
    {
        Skybox* skybox = Skybox::get_instance();
        util::set_int(cubemap.c_str(), skybox->cubmap_unit(), program);
        util::set_float(intensity.c_str(), skybox->intensity(), program);
        util::set_bool(use.c_str(), true, program);
        util::set_bool(affect_scene.c_str(), skybox->affect_scene(), program);
    }
    else util::set_bool(use.c_str(), false, program);
}

Skybox* Skybox::singleton = nullptr;