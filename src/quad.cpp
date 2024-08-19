#include "quad.h"

#include "globals.h"
#include "camera.h"
#include "shader.h"
#include "model.h"
#include "fboManager.h"
#include "material.h"


PostprocQuad* PostprocQuad::instance = nullptr;

PostprocQuad *PostprocQuad::getInstance()
{
    if(!instance) return new PostprocQuad();
    else return instance;
}

void PostprocQuad::drawOnScreen(const Shader::PostProc& shader) const
{
    assert(shader.shaderName() == "./shader/postQuad");
    if(util::Globals::bloom)
        runBlur(*Shader::BloomBlur::getInstance(), util::Globals::blurBrightImageUnit);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shader.use();
    shader.setUniforms(util::Globals::deferred ? Model::getTexture("sceneColorAttachments[0]").texUnit : Model::getTexture("sceneColorAttachmentsMS[0]").texUnit, util::Globals::blurBrightImageUnit);
    glBindVertexArray(quadVAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Dump depth information from sceneMSFBO/sceneFBO to default fbo
    glBindFramebuffer(GL_READ_FRAMEBUFFER, util::Globals::deferred ? FBOManager::FBOData::windowsSizeFBOs["sceneFBO"].fbo : FBOManager::FBOData::windowsSizeFBOs["sceneMSFBO"].fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, SRC_WIDTH, util::Globals::camera.height(),
        0, 0, SRC_WIDTH, util::Globals::camera.height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindVertexArray(0);
}

void PostprocQuad::lightingPass(const Shader::LightingPass &shader, const Camera &camera, GLuint fbo) const
{
    assert(util::Globals::pbrMat ? shader.shaderName() == "./shader/lightingPassPBR" : shader.shaderName() == "./shader/lightingPassBP");
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindVertexArray(quadVAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.setUniforms(camera);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Dump depth information from GBuffer fbo to current sceneMSFBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBOManager::FBOData::windowsSizeFBOs["GemometryFBO"].fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, SRC_WIDTH, util::Globals::camera.height(),
        0, 0, SRC_WIDTH, util::Globals::camera.height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostprocQuad::SSAOPass(const Shader::SSAO &shader, const Shader::SSAOBlur &blur_shader, GLuint ssao_fbo, GLuint fbo) const
{
    assert(shader.shaderName() == "./shader/ssao");
    assert(blur_shader.shaderName() == "./shader/ssaoBlur");
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
    glBindVertexArray(quadVAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.set_uniforms();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    blur_shader.use();
    blur_shader.setUniforms();
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

    glBindVertexArray(quadVAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.setUniforms(Model::getTexture("accumTex").texUnit, Model::getTexture("revealageTex").texUnit);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostprocQuad::runBlur(const Shader::BloomBlur &shader, GLuint &blurBufferTexUnit) const
{
    assert(shader.shaderName() == "./shader/bloomBlur");
    if(!util::Globals::bloom) 
    {
        blurBufferTexUnit = -1;
        return;
    }
    bool horizental = true;
    int amount = 10;
    assert((amount & 1) == 0);  // Ensure that blur operates even times

    std::string FBONamePrex = util::Globals::deferred ? "pingpongFBO" : "pingpongFBOMS";
    for(int i = 0; i < amount; ++i)
    {
        // 现在要blur的结果要存在哪个fbo的color attachment中
        glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::FBOData::windowsSizeFBOs[(FBONamePrex + "[" + std::to_string(int(horizental)) + "]").c_str()].fbo);
        glBindVertexArray(quadVAO);
        for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
        shader.use();
        // 现在要blur哪个fbo的color attachment
        GLuint blurTexUnit = util::Globals::deferred ? 
            (i == 0 ? Model::getTexture("sceneColorAttachments[1]").texUnit : Model::getTexture(("pingpongColorAttchment[" + std::to_string(int(!horizental)) + "]").c_str()).texUnit) : 
            (i == 0 ? Model::getTexture("sceneColorAttachmentsMS[1]").texUnit : Model::getTexture(("pingpongColorAttchmentMS[" + std::to_string(int(!horizental)) + "]").c_str()).texUnit);
        shader.setUniforms(blurTexUnit, horizental);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizental = !horizental;
    }
    blurBufferTexUnit = util::Globals::deferred ? Model::getTexture("pingpongColorAttchment[0]").texUnit : Model::getTexture("pingpongColorAttchmentMS[0]").texUnit;

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
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quad_VBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(3 * sizeof(float)));
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PostprocQuad::TAAResolvePass(const Shader::TAAResolve& shader, GLuint fbo) const
{
    assert(shader.shaderName() == "./shader/TAAResolve");
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDepthFunc(GL_ALWAYS);

    glBindVertexArray(quadVAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    // 给两帧的容错，要不然会出问题
    shader.setUniforms(Model::getTexture("sceneColorAttachments[0]").texUnit, 
        util::Globals::deferedFrameIndex <= 2 ? Model::getTexture("sceneColorAttachments[0]").texUnit : Model::getTexture("historyColor").texUnit);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDepthFunc(GL_LESS);

    // Blit historyColor to sceneColorAttachments[0]
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBOManager::FBOData::windowsSizeFBOs["sceneFBO"].fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);  // historyColor
    glDrawBuffer(GL_COLOR_ATTACHMENT0);  // sceneColorAttachments[0]
    glBlitFramebuffer(0, 0, SRC_WIDTH, SRC_HEIGHT, 0, 0, SRC_WIDTH, SRC_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Remember to set drawbuffers back to sceneColorAttachments[0] and sceneColorAttachments[1]!
    unsigned int drawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}