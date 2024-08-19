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

void Mesh::forwardPass(const Shader::ObjectShader& shader, const Camera& camera, const glm::mat4& model, GLuint fbo) const
{
    if(m_material->matType() == Mat_type::Blinn_Phong)
        assert(shader.shaderName() == "./shader/blinnPhong");
    else if(m_material->matType() == Mat_type::PBR)
        assert(shader.shaderName() == "./shader/physicallyBased");
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
    // I have sperated rendering of the transparent objects into a transparent pass, so the below code won't execute.
    if(m_material->isTransparent() && m_isblend)
    {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    }
    if(m_iscullface)
        glEnable(GL_CULL_FACE);

    glBindVertexArray(VAO);
    for(int i = 0; i < 4; ++i) glEnableVertexAttribArray(i);
    shader.setUniforms(*m_material, camera, model);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);

    // Render settings back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);
}

void Mesh::gbufferPass(const Shader::GBuffer &shader, const Camera &camera, const glm::mat4 &model, GLuint fbo) const
{
    assert(shader.shaderName() == (util::Globals::pbrMat ? "./shader/GBufferPBR" : "./shader/GBufferBP"));
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

bool Mesh::isTransparent() const
{
    return m_material->isTransparent();
}

void Mesh::forwardNormals(const Shader::Normal& shader, const Camera& camera, const glm::mat4& model) const
{
    assert(shader.shaderName() == "./shader/normal");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(VAO_normal);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.setUniforms(camera, model);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
}

void Mesh::forwardTangent(const Shader::TangentNormal &shader, const Camera &camera, const glm::mat4 &model) const
{
    assert(shader.shaderName() == "./shader/tangentNormal");
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

void Mesh::draw_outline(const Shader::SingleColor& shader, const Camera& camera, const glm::mat4& model) const
{
    assert(shader.shaderName() == "./shader/singleColor");
    if(!m_outlined) return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    shader.use();
    glBindVertexArray(VAO);
    shader.setUniforms(model, {1.f, .5f, .25f});
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);

    glDisable(GL_STENCIL_TEST);
    glBindVertexArray(0);
}

void Mesh::depthmapPass(const Shader::DepthShader& depth_shader, const Light& light, const glm::mat4& model) const
{
    if(light.type() == Light_type::SUN) assert(depth_shader.shaderName() == "./shader/cascadeMap");
    else if(light.type() == Light_type::POINT) assert(depth_shader.shaderName() == "./shader/depthCubemap");
    glEnable(GL_DEPTH_TEST);
    depth_shader.use();
    depth_shader.set_uniforms(model, light);
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
    
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}

void Mesh::transparentPass(const Shader::TransparentWBPBR& transShader, const Camera& camera, const glm::mat4& model, GLuint fbo) const
{
    assert(transShader.shaderName() == "./shader/transparentWBPBR");
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);  // disable depth writing!
    glEnable(GL_BLEND);
    glBlendFunci(0, GL_ONE, GL_ONE);
    glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

    transShader.use();
    transShader.setUniforms(*m_material, camera, model);
    glBindVertexArray(VAO);
    for(int i = 0; i < 4; ++i) glEnableVertexAttribArray(i);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);

    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
