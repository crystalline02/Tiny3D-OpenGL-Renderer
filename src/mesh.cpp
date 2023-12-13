#include "mesh.h"
#include "globals.h"
#include "shader.h"
#include "camera.h"
#include "material.h"
#include "light.h"

#include <glad/glad.h>

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, Material* material):
m_vertices(vertices), m_indices(indices), m_material(material), m_isblend(false), m_iscullface(false), m_outlined(false)
{
    m_center = glm::vec3{0.f};
    unsigned int num_vertices = vertices.size();
    for(Vertex& v: m_vertices)
        m_center += v.positon / float(num_vertices);
    setup_mesh();
}

Mesh::~Mesh()
{
    clear();
    delete m_material;
}

void Mesh::setup_mesh()
{
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &VAO_normal);
    glGenVertexArrays(1, &VAO_tangent);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), &m_vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indices.size(), &m_indices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tangent)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texture_coords)));
    for(int i = 0; i < 4; ++i) glEnableVertexAttribArray(i);
    
    glBindVertexArray(VAO_normal);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);

    glBindVertexArray(VAO_tangent);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tangent)));
    for(int i = 0; i < 3; ++i) glEnableVertexAttribArray(i);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::draw(const Shader::Object_shader& shader, const Camera& camera, const glm::mat4& model, GLuint fbo) const
{
    if(m_material->mat_type() == Mat_type::Blinn_Phong)
        assert(shader.shader_dir() == "./shader/blinn_phong");
    else if(m_material->mat_type() == Mat_type::PBR)
        assert(shader.shader_dir() == "./shader/physically_based");
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    shader.use();
    // Render settings
    if(m_outlined)
    {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
        glStencilMask(0xFF);
    }
    glEnable(GL_DEPTH_TEST);
    if(m_material->is_blend_mat() && m_isblend)
    {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    }
    if(m_iscullface)
        glEnable(GL_CULL_FACE);

    glBindVertexArray(VAO);
    for(int i = 0; i < 4; ++i) glEnableVertexAttribArray(i);
    shader.set_uniforms(*m_material, camera, model);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);

    // Render settings back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glBindVertexArray(0);
}

void Mesh::draw_gbuffer(const Shader::G_buffer &shader, const Camera &camera, const glm::mat4 &model, GLuint fbo) const
{
    assert(shader.shader_dir() == "./shader/G_buffer");
    assert(fbo != 0);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(VAO);
    for(int i = 0; i < 4; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.set_uniforms(*m_material, camera, model);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);

    // Render settings back to default
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

bool Mesh::is_blend_mesh() const
{
    return m_material->is_blend_mat();
}

void Mesh::draw_normals(const Shader::Normal& shader, const Camera& camera, const glm::mat4& model) const
{
    assert(shader.shader_dir() == "./shader/normal");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(VAO_normal);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.set_uniforms(camera, model);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
}

void Mesh::draw_tangent(const Shader::Tangent_normal &shader, const Camera &camera, const glm::mat4 &model) const
{
    assert(shader.shader_dir() == "./shader/tangent_normal");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(VAO_tangent);
    for(int i = 0; i < 3; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.set_uniforms(camera, model);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
}

void Mesh::draw_outline(const Shader::Single_color& shader, const Camera& camera, const glm::mat4& model) const
{
    assert(shader.shader_dir() == "./shader/single_color");
    if(!m_outlined) return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    shader.use();
    glBindVertexArray(VAO);
    shader.set_uniforms(model, {1.f, .5f, .25f});
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);

    glDisable(GL_STENCIL_TEST);
    glBindVertexArray(0);
}

void Mesh::draw_depthmap(const Shader::Depth_shader& depth_shader, const Light& light, const glm::mat4& model) const
{
    if(light.type() == Light_type::SUN) assert(depth_shader.shader_dir() == "./shader/cascade_map");
    else if(light.type() == Light_type::POINT) assert(depth_shader.shader_dir() == "./shader/depth_cubemap");
    glEnable(GL_DEPTH_TEST);
    depth_shader.use();
    depth_shader.set_uniforms(model, light);
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
    
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}

void Mesh::clear()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO_normal);
    glDeleteVertexArrays(1, &VAO_tangent);
}

void Mesh::switch_mat_type(Mat_type new_type)
{
    m_material->switch_mat_type(new_type);
}
