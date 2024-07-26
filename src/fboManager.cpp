#include "fboManager.h"

#include "material.h"
#include "model.h"
#include "globals.h"
#include "camera.h"

std::unordered_map<std::string, FBOManager::FBOData> FBOManager::FBOData::windows_sized_fbos;

void FBOManager::genWindowFBOs()
{
    createSceneFramebufferMS();
    createPingpongFramebufferMS();
    createGFrambuffer();
    createSSAOFramebuffer();
    createTransparentFBO();
}

void FBOManager::regenWindowFBOs()
{
    for(auto it = FBOData::windows_sized_fbos.begin(); it != FBOData::windows_sized_fbos.end(); ++it)
    {
        glDeleteFramebuffers(1, &it->second.fbo);
        glDeleteRenderbuffers(1, &it->second.rbo);
        for(const Texture& tex : it->second.textures)
            Model::rm_texture(tex);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, util::Globals::camera.width(), util::Globals::camera.height(), 0, GL_RGBA, GL_HALF_FLOAT,nullptr);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, util::Globals::camera.width(), util::Globals::camera.height(), 0, GL_RED, GL_FLOAT, nullptr);
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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, util::Globals::camera.width(), util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, transparentFBOData.rbo);

    GLuint colorAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, colorAttachments);

    util::checkFrameBufferInComplete("transparentFBO");
    FBOManager::FBOData::windows_sized_fbos["transparentFBO"] = transparentFBOData;

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FBOManager::createGFrambuffer()
{
    FBOData fbo_struct;
    Texture postionTex, normalDepthTex, surfaceNormalTex, albedoSpecularTex, ambientTex;

    glGenFramebuffers(1, &fbo_struct.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct.fbo);

    postionTex.texName = "G buffer position buffer";
    glGenTextures(1, &postionTex.texBuffer);
    postionTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(postionTex.texName.c_str(), postionTex);
    glActiveTexture(GL_TEXTURE0 + postionTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, postionTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postionTex.texBuffer, 0);
    fbo_struct.textures.push_back(postionTex);

    normalDepthTex.texName = "G buffer normal_depth buffer";
    glGenTextures(1, &normalDepthTex.texBuffer);
    normalDepthTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(normalDepthTex.texName.c_str(), normalDepthTex);
    glActiveTexture(GL_TEXTURE0 + normalDepthTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, normalDepthTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalDepthTex.texBuffer, 0);
    fbo_struct.textures.push_back(normalDepthTex);

    surfaceNormalTex.texName = "G buffer surface normal buffer";
    glGenTextures(1, &surfaceNormalTex.texBuffer);
    surfaceNormalTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(surfaceNormalTex.texName.c_str(), surfaceNormalTex);
    glActiveTexture(GL_TEXTURE0 + surfaceNormalTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, surfaceNormalTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, surfaceNormalTex.texBuffer, 0);
    fbo_struct.textures.push_back(surfaceNormalTex);

    albedoSpecularTex.texName = "G buffer albedo_specular buffer";
    glGenTextures(1, &albedoSpecularTex.texBuffer);
    albedoSpecularTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(albedoSpecularTex.texName.c_str(), albedoSpecularTex);
    glActiveTexture(GL_TEXTURE0 + albedoSpecularTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, albedoSpecularTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, albedoSpecularTex.texBuffer, 0);
    fbo_struct.textures.push_back(albedoSpecularTex);

    ambientTex.texName = "G buffer ambient_metalic buffer";
    glGenTextures(1, &ambientTex.texBuffer);
    ambientTex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(ambientTex.texName.c_str(), ambientTex);
    glActiveTexture(GL_TEXTURE0 + ambientTex.texUnit);
    glBindTexture(GL_TEXTURE_2D, ambientTex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, ambientTex.texBuffer, 0);
    fbo_struct.textures.push_back(ambientTex);

    glGenRenderbuffers(1, &fbo_struct.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo_struct.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, util::Globals::camera.width(), util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo_struct.rbo);

    util::checkFrameBufferInComplete("GBuffer");

    FBOManager::FBOData::windows_sized_fbos["Gemometry fbo"] = fbo_struct;

    unsigned int color_attachments[5] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
    glDrawBuffers(5, color_attachments);

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FBOManager::createPingpongFramebufferMS()
{
    FBOData fbo_struct[2];
    Texture pingpong_texs[2];
    
    for(int i = 0; i < 2; ++i)
    {
        glGenFramebuffers(1, &fbo_struct[i].fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct[i].fbo);

        pingpong_texs[i].texName = "pingpong color attchment[" + std::to_string(i) + "]";
        glGenTextures(1, &pingpong_texs[i].texBuffer);
        pingpong_texs[i].texUnit = Model::fetchNewTexunit();
        Model::add_texture(pingpong_texs[i].texName.c_str(), pingpong_texs[i]);
        glActiveTexture(GL_TEXTURE0 + pingpong_texs[i].texUnit);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, pingpong_texs[i].texBuffer);
        // pingpong fbo的color buffer需要16bit的，这是因为我们主要用pingpong fbo来做blur，初始给pingpong fbo
        // 的birghtness image就是16bit的（scene fbo），最终blur的结果也应该是16bit的
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F,
            util::Globals::camera.width(), util::Globals::camera.height(), GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, pingpong_texs[i].texBuffer, 0);
        // Note: no need for depth or stencil attachment as what we only care about is color buffer
        fbo_struct[i].textures.push_back(pingpong_texs[i]);

        fbo_struct[i].rbo = 0;
        
        util::checkFrameBufferInComplete("PingPongFrameBuffer");
        FBOManager::FBOData::windows_sized_fbos["pingpng fbo[" + std::to_string(i) + "]"] = fbo_struct[i];
    }
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBOManager::createSSAOFramebuffer()
{
    FBOData fbo_struct_ssao, fbo_struct_ssao_blur;
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
    glGenFramebuffers(1, &fbo_struct_ssao.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct_ssao.fbo);
    
    ssao_tex.texName = "SSAO color buffer";
    glGenTextures(1, &ssao_tex.texBuffer);
    ssao_tex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(ssao_tex.texName.c_str(), ssao_tex);
    glActiveTexture(GL_TEXTURE0 + ssao_tex.texUnit);
    glBindTexture(GL_TEXTURE_2D, ssao_tex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_tex.texBuffer, 0);
    fbo_struct_ssao.textures.push_back(ssao_tex);

    fbo_struct_ssao.rbo = 0;

    FBOManager::FBOData::windows_sized_fbos["SSAO fbo"] = fbo_struct_ssao;

    // create ssao_blur framebuffer
    glGenFramebuffers(1, &fbo_struct_ssao_blur.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct_ssao_blur.fbo);

    ssao_blur_tex.texName = "SSAO blur buffer";
    glGenTextures(1, &ssao_blur_tex.texBuffer);
    ssao_blur_tex.texUnit = Model::fetchNewTexunit();
    Model::add_texture(ssao_blur_tex.texName.c_str(), ssao_blur_tex);
    glActiveTexture(GL_TEXTURE0 + ssao_blur_tex.texUnit);
    glBindTexture(GL_TEXTURE_2D, ssao_blur_tex.texBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, util::Globals::camera.width(), util::Globals::camera.height(), 0,
        GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_tex.texBuffer, 0);
    fbo_struct_ssao_blur.textures.push_back(ssao_blur_tex);

    fbo_struct_ssao_blur.rbo = 0;

    FBOManager::FBOData::windows_sized_fbos["SSAO blur fbo"] = fbo_struct_ssao_blur;

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBOManager::createSceneFramebufferMS()
{
    FBOData fbo_struct;

    glGenFramebuffers(1, &fbo_struct.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct.fbo);

    Texture color_attachments_tex[2];
    glGenTextures(1, &color_attachments_tex[0].texBuffer);
    glGenTextures(1, &color_attachments_tex[1].texBuffer);
    for(unsigned int i = 0; i < 2; ++i)
    {
        color_attachments_tex[i].texUnit = Model::fetchNewTexunit();
        color_attachments_tex[i].texName = "scene fbo color attachments[" + std::to_string(i) +"]";
        Model::add_texture(color_attachments_tex[i].texName.c_str(), color_attachments_tex[i]);
        glActiveTexture(GL_TEXTURE0 + color_attachments_tex[i].texUnit);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color_attachments_tex[i].texBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F,
            util::Globals::camera.width(), util::Globals::camera.height(), GL_TRUE);
        // It's invalid to set below texture paramerters for GL_TEXTURE_2D_MULTISAMPLE, otherwise GL_INVALID_ENUM error will be raised.
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, color_attachments_tex[i].texBuffer, 0);
        fbo_struct.textures.push_back(color_attachments_tex[i]);
    }

    glGenRenderbuffers(1, &fbo_struct.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo_struct.rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8,
        util::Globals::camera.width(), util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo_struct.rbo);

    FBOManager::FBOData::windows_sized_fbos["scene ms fbo"] = fbo_struct;

    // Announcing that we will draw on 2 color attachments
    unsigned int draw_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, draw_buffers);
    util::checkFrameBufferInComplete("sceneMsFrmaeBuffer");

    glActiveTexture(GL_TEXTURE0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBOManager::clearBuffers()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);  // 设置ClearColor的值
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);  // 使用ClearColor的值来clearGL_COLOR_BUFFER_BIT,GL_DEPTH_BUFFER_BIT默认为1.f，GL_STENCIL_BUFFER_BIT默认为0

	glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::FBOData::windows_sized_fbos["scene ms fbo"].fbo);
	glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::FBOData::windows_sized_fbos["transparentFBO"].fbo);
    glm::vec4 zeroVec(0.f, 0.f, 0.f, 0.f), oneVec(1.f, 1.f, 1.f, 1.f);
	glClearBufferfv(GL_COLOR, 0, &zeroVec[0]);  // clear accum
	glClearBufferfv(GL_COLOR, 1, &oneVec[0]);  // clear revealage
    glClear(GL_DEPTH_BUFFER_BIT);  // clear depth

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if(util::Globals::deferred_rendering)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::FBOData::windows_sized_fbos["Gemometry fbo"].fbo);
		glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}