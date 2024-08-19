#include "fboManager.h"

#include "material.h"
#include "model.h"
#include "globals.h"
#include "camera.h"
#include "TAAManager.h"

#include <limits>

std::unordered_map<std::string, FBOManager::FBOData> FBOManager::FBOData::windowsSizeFBOs;

void FBOManager::genWindowFBOs()
{
    createSceneFramebufferMS();
    createSceneFramebuffer();
    createHistoryFramebuffer();
    createPingpongFramebufferMS();
    createPingpongFramebuffer();
    createGFrambuffer();
    createSSAOFramebuffer();
    createTransparentFBO();
}

void FBOManager::regenWindowFBOs()
{
    for(auto it = FBOData::windowsSizeFBOs.begin(); it != FBOData::windowsSizeFBOs.end(); ++it)
    {
        glDeleteFramebuffers(1, &it->second.fbo);
        glDeleteRenderbuffers(1, &it->second.rbo);
        for(const Texture& tex : it->second.textures)
            Model::rmTexture(tex);
    }
    genWindowFBOs();
}

void FBOManager::createTransparentFBO()
{
    FBOData transparentFBOData;
    Texture accumTex, revealage;

    glGenFramebuffers(1, &transparentFBOData.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, transparentFBOData.fbo);

    accumTex.texName = "accumTex";
    glGenTextures(1, &accumTex.texBuffer);
    accumTex.texUnit = Model::fetchNewTexunit();
    glActiveTexture(GL_TEXTURE0 + accumTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, accumTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SRC_WIDTH, util::Globals::camera.height(), 0, GL_RGBA, GL_HALF_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    Model::add_texture(accumTex.texName.c_str(), accumTex);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTex.texBuffer, 0);
    transparentFBOData.textures.push_back(accumTex);
    
    revealage.texName = "revealageTex";
    glGenTextures(1, &revealage.texBuffer);
    revealage.texUnit = Model::fetchNewTexunit();
    glActiveTexture(GL_TEXTURE0 + revealage.texUnit);
    glBindTexture(GL_TEXTURE_2D, revealage.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, SRC_WIDTH, util::Globals::camera.height(), 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    Model::add_texture(revealage.texName.c_str(), revealage);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, revealage.texBuffer, 0);
    transparentFBOData.textures.push_back(revealage);

    // Depth is needed as we will do depth testing based on the previous opaque pass.The depth infomation is stored in "sceme ms fbo".
    // Use glBlitFrameBuffer to get the depth infomation.
    glGenRenderbuffers(1, &transparentFBOData.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, transparentFBOData.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SRC_WIDTH, util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, transparentFBOData.rbo);

    GLuint colorAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, colorAttachments);

    util::checkFrameBufferInComplete("transparentFBO");
    FBOManager::FBOData::windowsSizeFBOs["transparentFBO"] = transparentFBOData;

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FBOManager::createGFrambuffer()
{
    FBOData GBufferFBO;
    Texture postionTex, normalDepthTex, surfaceNormalTex, albedoSpecularTex, ambientMetalicTex, motionVecTex;

    glGenFramebuffers(1, &GBufferFBO.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, GBufferFBO.fbo);

    postionTex.texName = "GBufferPostionAlpha";
    glGenTextures(1, &postionTex.texBuffer);
    postionTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(postionTex.texName.c_str(), postionTex);
    glActiveTexture(GL_TEXTURE0 + postionTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, postionTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SRC_WIDTH, util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postionTex.texBuffer, 0);
    GBufferFBO.textures.push_back(postionTex);

    normalDepthTex.texName = "GBufferNormalDepth";
    glGenTextures(1, &normalDepthTex.texBuffer);
    normalDepthTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(normalDepthTex.texName.c_str(), normalDepthTex);
    glActiveTexture(GL_TEXTURE0 + normalDepthTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, normalDepthTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SRC_WIDTH, util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalDepthTex.texBuffer, 0);
    GBufferFBO.textures.push_back(normalDepthTex);

    surfaceNormalTex.texName = "GBufferSurfaceNormal";
    glGenTextures(1, &surfaceNormalTex.texBuffer);
    surfaceNormalTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(surfaceNormalTex.texName.c_str(), surfaceNormalTex);
    glActiveTexture(GL_TEXTURE0 + surfaceNormalTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, surfaceNormalTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SRC_WIDTH, util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, surfaceNormalTex.texBuffer, 0);
    GBufferFBO.textures.push_back(surfaceNormalTex);

    albedoSpecularTex.texName = "GBufferAlbedoSpecular";
    glGenTextures(1, &albedoSpecularTex.texBuffer);
    albedoSpecularTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(albedoSpecularTex.texName.c_str(), albedoSpecularTex);
    glActiveTexture(GL_TEXTURE0 + albedoSpecularTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, albedoSpecularTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SRC_WIDTH, util::Globals::camera.height(),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, albedoSpecularTex.texBuffer, 0);
    GBufferFBO.textures.push_back(albedoSpecularTex);

    ambientMetalicTex.texName = "GBufferAmbientMetalic";
    glGenTextures(1, &ambientMetalicTex.texBuffer);
    ambientMetalicTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(ambientMetalicTex.texName.c_str(), ambientMetalicTex);
    glActiveTexture(GL_TEXTURE0 + ambientMetalicTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, ambientMetalicTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SRC_WIDTH, util::Globals::camera.height(),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, ambientMetalicTex.texBuffer, 0);
    GBufferFBO.textures.push_back(ambientMetalicTex);

    motionVecTex.texName = "GBufferMotionVec";
    glGenTextures(1, &motionVecTex.texBuffer);
    motionVecTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(motionVecTex.texName.c_str(), motionVecTex);
    glActiveTexture(GL_TEXTURE0 + motionVecTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, motionVecTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, SRC_WIDTH, SRC_HEIGHT,
        0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, motionVecTex.texBuffer, 0);
    GBufferFBO.textures.emplace_back(motionVecTex);

    glGenRenderbuffers(1, &GBufferFBO.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, GBufferFBO.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SRC_WIDTH, util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, GBufferFBO.rbo);

    util::checkFrameBufferInComplete("GBuffer");

    FBOManager::FBOData::windowsSizeFBOs["GemometryFBO"] = GBufferFBO;

    unsigned int color_attachments[5] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
    glDrawBuffers(5, color_attachments);

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FBOManager::createPingpongFramebuffer()
{
    FBOData fboStruct[2];
    Texture pingpongTexs[2];
    
    for(int i = 0; i < 2; ++i)
    {
        glGenFramebuffers(1, &fboStruct[i].fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fboStruct[i].fbo);

        pingpongTexs[i].texName = "pingpongColorAttchment[" + std::to_string(i) + "]";
        glGenTextures(1, &pingpongTexs[i].texBuffer);
        pingpongTexs[i].texUnit = Model::fetchNewTexunit();
        Model::add_texture(pingpongTexs[i].texName.c_str(), pingpongTexs[i]);
        glActiveTexture(GL_TEXTURE0 + pingpongTexs[i].texUnit);
        glBindTexture(GL_TEXTURE_2D, pingpongTexs[i].texBuffer);
        // pingpong fbo的color buffer需要16bit的，这是因为我们主要用pingpong fbo来做blur，初始给pingpong fbo
        // 的birghtness image就是16bit的（scene fbo），最终blur的结果也应该是16bit的
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
            SRC_WIDTH, util::Globals::camera.height(), 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongTexs[i].texBuffer, 0);
        // Note: no need for depth or stencil attachment as what we only care about is color buffer
        fboStruct[i].textures.push_back(pingpongTexs[i]);

        fboStruct[i].rbo = std::numeric_limits<GLuint>::max();
        
        util::checkFrameBufferInComplete("PingPongFrameBuffer");
        FBOManager::FBOData::windowsSizeFBOs["pingpongFBO[" + std::to_string(i) + "]"] = fboStruct[i];
    }
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBOManager::createPingpongFramebufferMS()
{
    FBOData fbo_struct[2];
    Texture pingpong_texs[2];
    
    for(int i = 0; i < 2; ++i)
    {
        glGenFramebuffers(1, &fbo_struct[i].fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct[i].fbo);

        pingpong_texs[i].texName = "pingpongColorAttchmentMS[" + std::to_string(i) + "]";
        glGenTextures(1, &pingpong_texs[i].texBuffer);
        pingpong_texs[i].texUnit = Model::fetchNewTexunit();
        Model::add_texture(pingpong_texs[i].texName.c_str(), pingpong_texs[i]);
        glActiveTexture(GL_TEXTURE0 + pingpong_texs[i].texUnit);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, pingpong_texs[i].texBuffer);
        // pingpong fbo的color buffer需要16bit的，这是因为我们主要用pingpong fbo来做blur，初始给pingpong fbo
        // 的birghtness image就是16bit的（scene fbo），最终blur的结果也应该是16bit的
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F,
            SRC_WIDTH, util::Globals::camera.height(), GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, pingpong_texs[i].texBuffer, 0);
        // Note: no need for depth or stencil attachment as what we only care about is color buffer
        fbo_struct[i].textures.push_back(pingpong_texs[i]);

        fbo_struct[i].rbo = std::numeric_limits<GLuint>::max();
        
        util::checkFrameBufferInComplete("PingPongFrameBufferMS");
        FBOManager::FBOData::windowsSizeFBOs["pingpongFBOMS[" + std::to_string(i) + "]"] = fbo_struct[i];
    }
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBOManager::createSSAOFramebuffer()
{
    FBOData SSAOFBO, SSAOBlurFBO;
    Texture ssao_tex, ssao_blur_tex;

    // create ssao_noisetex used to generate random vector to generate tiled TBN matrix for placing the ssao samples
    std::array<glm::vec3, 16> rand_vector;
    std::uniform_real_distribution<float> rand(0.f, 1.f);
    std::default_random_engine generator;
    for(int i = 0; i < 16; ++i)
    {
        rand_vector[i] = glm::vec3(rand(generator) * 2.f - 1.f,
            rand(generator) * 2.f - 1.f,
            0.f);
    }

    if(Model::getTexture("SSAO noisetex").texName == "NULL texture")
    {
        Texture noise_tex;
        noise_tex.texName = "SSAO noisetex";
        glGenTextures(1, &noise_tex.texBuffer);
        noise_tex.texUnit = Model::fetchNewTexunit();
        Model::add_texture(noise_tex.texName.c_str(), noise_tex);
        glActiveTexture(GL_TEXTURE0 + noise_tex.texUnit);
        glBindTexture(GL_TEXTURE_2D, noise_tex.texBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &rand_vector[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    // create ssao framebuffer
    glGenFramebuffers(1, &SSAOFBO.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, SSAOFBO.fbo);
    
    ssao_tex.texName = "SSAO color buffer";
    glGenTextures(1, &ssao_tex.texBuffer);
    ssao_tex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(ssao_tex.texName.c_str(), ssao_tex);
    glActiveTexture(GL_TEXTURE0 + ssao_tex.texUnit);
    glBindTexture(GL_TEXTURE_2D, ssao_tex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SRC_WIDTH, util::Globals::camera.height(),
        0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_tex.texBuffer, 0);
    SSAOFBO.textures.push_back(ssao_tex);

    SSAOFBO.rbo = std::numeric_limits<GLuint>::max();

    util::checkFrameBufferInComplete("SSABOFBO");
    FBOManager::FBOData::windowsSizeFBOs["SSAOFBO"] = SSAOFBO;

    // create ssao_blur framebuffer
    glGenFramebuffers(1, &SSAOBlurFBO.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, SSAOBlurFBO.fbo);

    ssao_blur_tex.texName = "SSAO blur buffer";
    glGenTextures(1, &ssao_blur_tex.texBuffer);
    ssao_blur_tex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(ssao_blur_tex.texName.c_str(), ssao_blur_tex);
    glActiveTexture(GL_TEXTURE0 + ssao_blur_tex.texUnit);
    glBindTexture(GL_TEXTURE_2D, ssao_blur_tex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SRC_WIDTH, util::Globals::camera.height(), 0,
        GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_tex.texBuffer, 0);
    SSAOBlurFBO.textures.push_back(ssao_blur_tex);

    SSAOBlurFBO.rbo = std::numeric_limits<GLuint>::max();

    util::checkFrameBufferInComplete("SSAOBlurFBO");
    FBOManager::FBOData::windowsSizeFBOs["SSAOBlurFBO"] = SSAOBlurFBO;

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBOManager::createSceneFramebufferMS()
{
    FBOData sceneFBO;

    glGenFramebuffers(1, &sceneFBO.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO.fbo);

    Texture colorAttachmentsTex[2];
    glGenTextures(1, &colorAttachmentsTex[0].texBuffer);
    glGenTextures(1, &colorAttachmentsTex[1].texBuffer);
    for(unsigned int i = 0; i < 2; ++i)
    {
        colorAttachmentsTex[i].texUnit = Model::fetchNewTexunit();
        colorAttachmentsTex[i].texName = "sceneColorAttachmentsMS[" + std::to_string(i) +"]";
        Model::add_texture(colorAttachmentsTex[i].texName.c_str(), colorAttachmentsTex[i]);
        glActiveTexture(GL_TEXTURE0 + colorAttachmentsTex[i].texUnit);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorAttachmentsTex[i].texBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F,
            SRC_WIDTH, util::Globals::camera.height(), GL_TRUE);
        // It's invalid to set below texture paramerters for GL_TEXTURE_2D_MULTISAMPLE, otherwise GL_INVALID_ENUM error will be raised.
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, colorAttachmentsTex[i].texBuffer, 0);
        sceneFBO.textures.push_back(colorAttachmentsTex[i]);
    }

    glGenRenderbuffers(1, &sceneFBO.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneFBO.rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8,
        SRC_WIDTH, util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneFBO.rbo);

    FBOManager::FBOData::windowsSizeFBOs["sceneMSFBO"] = sceneFBO;

    // Announcing that we will draw on 2 color attachments
    unsigned int draw_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, draw_buffers);
    util::checkFrameBufferInComplete("sceneMsFrameBuffer");

    glActiveTexture(GL_TEXTURE0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBOManager::createSceneFramebuffer()
{
    FBOData sceneFBO;

    glGenFramebuffers(1, &sceneFBO.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO.fbo);

    Texture colorAttachmentsTex[2];
    glGenTextures(1, &colorAttachmentsTex[0].texBuffer);
    glGenTextures(1, &colorAttachmentsTex[1].texBuffer);
    for(unsigned int i = 0; i < 2; ++i)
    {
        colorAttachmentsTex[i].texName = "sceneColorAttachments[" + std::to_string(i) +"]";
        colorAttachmentsTex[i].texUnit = Model::fetchNewTexunit();
        Model::add_texture(colorAttachmentsTex[i].texName.c_str(), colorAttachmentsTex[i]);
        glActiveTexture(GL_TEXTURE0 + colorAttachmentsTex[i].texUnit);
        glBindTexture(GL_TEXTURE_2D, colorAttachmentsTex[i].texBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SRC_WIDTH, util::Globals::camera.height(), 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorAttachmentsTex[i].texBuffer, 0);
        sceneFBO.textures.push_back(colorAttachmentsTex[i]);
    }

    glGenRenderbuffers(1, &sceneFBO.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneFBO.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SRC_WIDTH, util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneFBO.rbo);

    FBOData::windowsSizeFBOs["sceneFBO"] = sceneFBO;

    // Announcing that we will draw on 2 color attachments
    unsigned int draw_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, draw_buffers);
    util::checkFrameBufferInComplete("sceneFrameBuffer");

    glActiveTexture(GL_TEXTURE0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void FBOManager::clearBuffers()
{
    glm::vec4 zeroVec(0.f, 0.f, 0.f, 0.f), oneVec(1.f, 1.f, 1.f, 1.f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);  // 设置ClearColor的值
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);  // 使用ClearColor的值来clearGL_COLOR_BUFFER_BIT,GL_DEPTH_BUFFER_BIT默认为1.f，GL_STENCIL_BUFFER_BIT默认为0

	glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::FBOData::windowsSizeFBOs["sceneMSFBO"].fbo);
	glClearBufferfv(GL_COLOR, 0, glm::value_ptr(util::Globals::bg_color));  // clear sceneColorAttachmentsMS[0]
	glClearBufferfv(GL_COLOR, 1, glm::value_ptr(zeroVec));  // clear sceneColorAttachmentsMS[1]
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::FBOData::windowsSizeFBOs["sceneFBO"].fbo);
	glClearBufferfv(GL_COLOR, 0, glm::value_ptr(util::Globals::bg_color));  // clear sceneColorAttachments[0]
	glClearBufferfv(GL_COLOR, 1, glm::value_ptr(zeroVec));  // clear sceneColorAttachments[1]
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::FBOData::windowsSizeFBOs["transparentFBO"].fbo);
	glClearBufferfv(GL_COLOR, 0, &zeroVec[0]);  // clear accum
	glClearBufferfv(GL_COLOR, 1, &oneVec[0]);  // clear revealage
    glClear(GL_DEPTH_BUFFER_BIT);  // clear depth

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if(util::Globals::deferred)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::FBOData::windowsSizeFBOs["GemometryFBO"].fbo);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void FBOManager::createHistoryFramebuffer()
{
    Texture historyTexture;
    glGenFramebuffers(1, &TAAManager::historyFBO.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, TAAManager::historyFBO.fbo);
    
    glGenTextures(1, &historyTexture.texBuffer);
    historyTexture.texUnit = Model::fetchNewTexunit();
    glActiveTexture(GL_TEXTURE0 + historyTexture.texUnit);
    glBindTexture(GL_TEXTURE_2D, historyTexture.texBuffer);
    historyTexture.texName = "historyColor";
    Model::add_texture(historyTexture.texName.c_str(), historyTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SRC_WIDTH, util::Globals::camera.height(), 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    TAAManager::historyFBO.textures.emplace_back(historyTexture);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, historyTexture.texBuffer, 0);

    util::checkFrameBufferInComplete("historyFBO");

    // no need for depth render buffer object
    TAAManager::historyFBO.rbo = std::numeric_limits<GLuint>::max();

    FBOData::windowsSizeFBOs["historyFBO"] = TAAManager::historyFBO;

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}