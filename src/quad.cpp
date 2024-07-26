#include "quad.h"

#include "globals.h"
#include "camera.h"
#include "shader.h"
#include "model.h"
#include "fboManager.h"
#include "material.h"

PostprocQuad* PostprocQuad::instance = nullptr;

PostprocQuad *PostprocQuad::get_instance()
{
    if(!instance) return new PostprocQuad();
    else return instance;
}

void PostprocQuad::drawFinal(const Shader::PostProc& shader) const
{
    assert(shader.shaderName() == "./shader/postQuad");
    if(util::Globals::bloom)
        run_blur(*Shader::BloomBlur::get_instance(), util::Globals::blur_brightimage_unit);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shader.use();
    shader.set_uniforms(Model::getTexture("scene fbo color attachments[0]").texUnit, util::Globals::blur_brightimage_unit);
    glBindVertexArray(quad_VAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Dump depth information from scene ms fbo to default fbo
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBOManager::FBOData::windows_sized_fbos["scene ms fbo"].fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, util::Globals::camera.width(), util::Globals::camera.height(),
        0, 0, util::Globals::camera.width(), util::Globals::camera.height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindVertexArray(0);
}

void PostprocQuad::lightingPass(const Shader::LightingPass &shader, const Camera &camera, GLuint fbo) const
{
    assert(util::Globals::pbr_mat ? shader.shaderName() == "./shader/lightingPassPBR" : shader.shaderName() == "./shader/lightingPassBP");
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindVertexArray(quad_VAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.set_uniforms(camera);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Dump depth information from GBuffer fbo to current scene ms fbo
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBOManager::FBOData::windows_sized_fbos["Gemometry fbo"].fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, util::Globals::camera.width(), util::Globals::camera.height(),
        0, 0, util::Globals::camera.width(), util::Globals::camera.height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostprocQuad::SSAOPass(const Shader::SSAO &shader, const Shader::SSAOBlur &blur_shader, GLuint ssao_fbo, GLuint fbo) const
{
    assert(shader.shaderName() == "./shader/ssao");
    assert(blur_shader.shaderName() == "./shader/ssaoBlur");
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
    glBindVertexArray(quad_VAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.set_uniforms();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    blur_shader.use();
    blur_shader.set_uniforms();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

void PostprocQuad::compositePass(const Shader::Composite& shader, GLuint fbo) const
{
    assert(shader.shaderName() == "./shader/composite");
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // both for 'FragColor' and 'Bloom'.This is rational.

    glBindVertexArray(quad_VAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.setUniforms(Model::getTexture("accumTex").texUnit, Model::getTexture("revealageTex").texUnit);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostprocQuad::run_blur(const Shader::BloomBlur &shader, GLuint &blur_buffer_unit) const
{
    assert(shader.shaderName() == "./shader/bloomBlur");
    if(!util::Globals::bloom) 
    {
        blur_buffer_unit = -1;
        return;
    }
    bool horizental = true;
    int amount = 10;
    assert((amount & 1) == 0);
    for(int i = 0; i < amount; ++i)
    {
        // 现在要blur的结果要存在哪个fbo的color attachment中
        glBindFramebuffer(GL_FRAMEBUFFER, 
            FBOManager::FBOData::windows_sized_fbos["pingpng fbo[" + std::to_string(int(horizental)) + "]"].fbo);
        glBindVertexArray(quad_VAO);
        for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
        shader.use();
        // 现在要blur哪个fbo的color attachment
        shader.set_uniforms(i == 0 ? Model::getTexture("scene fbo color attachments[1]").texUnit : 
            Model::getTexture(("pingpong color attchment[" + std::to_string(int(!horizental)) + "]").c_str()).texUnit, horizental);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizental = !horizental;
    }
    blur_buffer_unit = Model::getTexture("pingpong color attchment[0]").texUnit;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

PostprocQuad::PostprocQuad()
{
    float quad_vertices[] = {
        // position    // UV
        -1.f, 1.f, 0.f, 0.f, 1.f,
        1.f, 1.f, 0.f, 1.f, 1.f,
        -1.f, -1.f, 0.f, 0.f, 0.f,
        
        1.f, 1.f, 0.f, 1.f, 1.f,
        -1.f, -1.f, 0.f, 0.f, 0.f,
        1.f, -1.f, 0.f, 1.f, 0.f
    };
    glGenVertexArrays(1, &quad_VAO);
    glGenBuffers(1, &quad_VBO);
    glBindVertexArray(quad_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(3 * sizeof(float)));
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}