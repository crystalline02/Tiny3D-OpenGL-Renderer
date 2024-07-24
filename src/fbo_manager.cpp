#include "fbo_manager.h"

#include "material.h"
#include "model.h"
#include "globals.h"
#include "camera.h"

std::unordered_map<std::string, FBO_Manager::FBO_Data> FBO_Manager::FBO_Data::windows_sized_fbos;

void FBO_Manager::gen_window_FBOs()
{
    create_scene_framebuffer_ms();
    create_pingpong_framebuffer_ms();
    create_G_frambuffer();
    create_ssao_framebuffer();
}

void FBO_Manager::regen_window_FBOs()
{
    for(auto it = FBO_Data::windows_sized_fbos.begin(); it != FBO_Data::windows_sized_fbos.end(); ++it)
    {
        glDeleteFramebuffers(1, &it->second.fbo);
        glDeleteRenderbuffers(1, &it->second.rbo);
        for(const Texture& tex : it->second.textures)
            Model::rm_texture(tex);
    }
    gen_window_FBOs();
}

void FBO_Manager::create_G_frambuffer()
{
    FBO_Data fbo_struct;
    Texture postion_tex, normal_depth_tex, surface_normal_tex, albedo_specular_tex, ambient_tex;

    glGenFramebuffers(1, &fbo_struct.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct.fbo);

    postion_tex.texname = "G buffer position buffer";
    glGenTextures(1, &postion_tex.texbuffer);
    postion_tex.texunit = Model::fatch_new_texunit();
    Model::add_texture(postion_tex.texname.c_str(), postion_tex);
    glActiveTexture(GL_TEXTURE0 + postion_tex.texunit);
    glBindTexture(GL_TEXTURE_2D, postion_tex.texbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postion_tex.texbuffer, 0);
    fbo_struct.textures.push_back(postion_tex);

    normal_depth_tex.texname = "G buffer normal_depth buffer";
    glGenTextures(1, &normal_depth_tex.texbuffer);
    normal_depth_tex.texunit = Model::fatch_new_texunit();
    Model::add_texture(normal_depth_tex.texname.c_str(), normal_depth_tex);
    glActiveTexture(GL_TEXTURE0 + normal_depth_tex.texunit);
    glBindTexture(GL_TEXTURE_2D, normal_depth_tex.texbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_depth_tex.texbuffer, 0);
    fbo_struct.textures.push_back(normal_depth_tex);

    surface_normal_tex.texname = "G buffer surface normal buffer";
    glGenTextures(1, &surface_normal_tex.texbuffer);
    surface_normal_tex.texunit = Model::fatch_new_texunit();
    Model::add_texture(surface_normal_tex.texname.c_str(), surface_normal_tex);
    glActiveTexture(GL_TEXTURE0 + surface_normal_tex.texunit);
    glBindTexture(GL_TEXTURE_2D, surface_normal_tex.texbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, surface_normal_tex.texbuffer, 0);
    fbo_struct.textures.push_back(surface_normal_tex);

    albedo_specular_tex.texname = "G buffer albedo_specular buffer";
    glGenTextures(1, &albedo_specular_tex.texbuffer);
    albedo_specular_tex.texunit = Model::fatch_new_texunit();
    Model::add_texture(albedo_specular_tex.texname.c_str(), albedo_specular_tex);
    glActiveTexture(GL_TEXTURE0 + albedo_specular_tex.texunit);
    glBindTexture(GL_TEXTURE_2D, albedo_specular_tex.texbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, albedo_specular_tex.texbuffer, 0);
    fbo_struct.textures.push_back(albedo_specular_tex);

    ambient_tex.texname = "G buffer ambient_metalic buffer";
    glGenTextures(1, &ambient_tex.texbuffer);
    ambient_tex.texunit = Model::fatch_new_texunit();
    Model::add_texture(ambient_tex.texname.c_str(), ambient_tex);
    glActiveTexture(GL_TEXTURE0 + ambient_tex.texunit);
    glBindTexture(GL_TEXTURE_2D, ambient_tex.texbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, ambient_tex.texbuffer, 0);
    fbo_struct.textures.push_back(ambient_tex);

    glGenRenderbuffers(1, &fbo_struct.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo_struct.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, util::Globals::camera.width(), util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo_struct.rbo);

    FBO_Manager::FBO_Data::windows_sized_fbos["Gemometry fbo"] = fbo_struct;

    util::checkFrameBufferInComplete("GBuffer");

    unsigned int color_attachments[5] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
    glDrawBuffers(5, color_attachments);

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FBO_Manager::create_pingpong_framebuffer_ms()
{
    FBO_Data fbo_struct[2];
    Texture pingpong_texs[2];
    
    for(int i = 0; i < 2; ++i)
    {
        glGenFramebuffers(1, &fbo_struct[i].fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct[i].fbo);

        pingpong_texs[i].texname = "pingpong color attchment[" + std::to_string(i) + "]";
        glGenTextures(1, &pingpong_texs[i].texbuffer);
        pingpong_texs[i].texunit = Model::fatch_new_texunit();
        Model::add_texture(pingpong_texs[i].texname.c_str(), pingpong_texs[i]);
        glActiveTexture(GL_TEXTURE0 + pingpong_texs[i].texunit);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, pingpong_texs[i].texbuffer);
        // pingpong fbo的color buffer需要16bit的，这是因为我们主要用pingpong fbo来做blur，初始给pingpong fbo
        // 的birghtness image就是16bit的（scene fbo），最终blur的结果也应该是16bit的
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F,
            util::Globals::camera.width(), util::Globals::camera.height(), GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, pingpong_texs[i].texbuffer, 0);
        // Note: no need for depth or stencil attachment as what we only care about is color buffer
        fbo_struct[i].textures.push_back(pingpong_texs[i]);

        fbo_struct[i].rbo = 0;
        
        util::checkFrameBufferInComplete("PingPongFrameBuffer");
        FBO_Manager::FBO_Data::windows_sized_fbos["pingpng fbo[" + std::to_string(i) + "]"] = fbo_struct[i];
    }
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO_Manager::create_ssao_framebuffer()
{
    FBO_Data fbo_struct_ssao, fbo_struct_ssao_blur;
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

    if(Model::get_texture("SSAO noisetex").texname == "NULL texture")
    {
        Texture noise_tex;
        noise_tex.texname = "SSAO noisetex";
        glGenTextures(1, &noise_tex.texbuffer);
        noise_tex.texunit = Model::fatch_new_texunit();
        Model::add_texture(noise_tex.texname.c_str(), noise_tex);
        glActiveTexture(GL_TEXTURE0 + noise_tex.texunit);
        glBindTexture(GL_TEXTURE_2D, noise_tex.texbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &rand_vector[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    // create ssao framebuffer
    glGenFramebuffers(1, &fbo_struct_ssao.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct_ssao.fbo);
    
    ssao_tex.texname = "SSAO color buffer";
    glGenTextures(1, &ssao_tex.texbuffer);
    ssao_tex.texunit = Model::fatch_new_texunit();
    Model::add_texture(ssao_tex.texname.c_str(), ssao_tex);
    glActiveTexture(GL_TEXTURE0 + ssao_tex.texunit);
    glBindTexture(GL_TEXTURE_2D, ssao_tex.texbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_tex.texbuffer, 0);
    fbo_struct_ssao.textures.push_back(ssao_tex);

    fbo_struct_ssao.rbo = 0;

    FBO_Manager::FBO_Data::windows_sized_fbos["SSAO fbo"] = fbo_struct_ssao;

    // create ssao_blur framebuffer
    glGenFramebuffers(1, &fbo_struct_ssao_blur.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct_ssao_blur.fbo);

    ssao_blur_tex.texname = "SSAO blur buffer";
    glGenTextures(1, &ssao_blur_tex.texbuffer);
    ssao_blur_tex.texunit = Model::fatch_new_texunit();
    Model::add_texture(ssao_blur_tex.texname.c_str(), ssao_blur_tex);
    glActiveTexture(GL_TEXTURE0 + ssao_blur_tex.texunit);
    glBindTexture(GL_TEXTURE_2D, ssao_blur_tex.texbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, util::Globals::camera.width(), util::Globals::camera.height(), 0,
        GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_tex.texbuffer, 0);
    fbo_struct_ssao_blur.textures.push_back(ssao_blur_tex);

    fbo_struct_ssao_blur.rbo = 0;

    FBO_Manager::FBO_Data::windows_sized_fbos["SSAO blur fbo"] = fbo_struct_ssao_blur;

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO_Manager::create_scene_framebuffer_ms()
{
    FBO_Data fbo_struct;

    glGenFramebuffers(1, &fbo_struct.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_struct.fbo);

    Texture color_attachments_tex[2];
    glGenTextures(1, &color_attachments_tex[0].texbuffer);
    glGenTextures(1, &color_attachments_tex[1].texbuffer);
    for(unsigned int i = 0; i < 2; ++i)
    {
        color_attachments_tex[i].texunit = Model::fatch_new_texunit();
        color_attachments_tex[i].texname = "scene fbo color attachments[" + std::to_string(i) +"]";
        Model::add_texture(color_attachments_tex[i].texname.c_str(), color_attachments_tex[i]);
        glActiveTexture(GL_TEXTURE0 + color_attachments_tex[i].texunit);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color_attachments_tex[i].texbuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F,
            util::Globals::camera.width(), util::Globals::camera.height(), GL_TRUE);
        // It's invalid to set below texture paramerters for GL_TEXTURE_2D_MULTISAMPLE, otherwise GL_INVALID_ENUM error will be raised.
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, color_attachments_tex[i].texbuffer, 0);
        fbo_struct.textures.push_back(color_attachments_tex[i]);
    }

    glGenRenderbuffers(1, &fbo_struct.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo_struct.rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8,
        util::Globals::camera.width(), util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo_struct.rbo);

    FBO_Manager::FBO_Data::windows_sized_fbos["scene ms fbo"] = fbo_struct;

    // Announcing that we will draw on 2 color attachments
    unsigned int draw_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, draw_buffers);
    util::checkFrameBufferInComplete("sceneMsFrmaeBuffer");

    glActiveTexture(GL_TEXTURE0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO_Manager::clear_buffers()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);  // 设置ClearColor的值
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);  // 使用ClearColor的值来clearGL_COLOR_BUFFER_BIT,GL_DEPTH_BUFFER_BIT默认为1.f，GL_STENCIL_BUFFER_BIT默认为0
	glBindFramebuffer(GL_FRAMEBUFFER, FBO_Manager::FBO_Data::windows_sized_fbos["scene ms fbo"].fbo);
	glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(util::Globals::deferred_rendering)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO_Manager::FBO_Data::windows_sized_fbos["Gemometry fbo"].fbo);
		glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}