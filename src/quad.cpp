#include "quad.h"
#include "globals.h"
#include "camera.h"
#include "shader.h"

Postproc_quad* Postproc_quad::instance = nullptr;

Postproc_quad *Postproc_quad::get_instance()
{
    if(!instance) return new Postproc_quad();
    else return instance;
}

void Postproc_quad::draw(const Post_proc& shader) const
{
    assert(shader.shader_dir() == "./shader/post_quad");
    if(util::Globals::bloom)
        run_blur(*Bloom_blur::get_instance(), util::Globals::blur_brightimage_unit);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shader.use();
    shader.set_uniforms(util::Globals::scene_unit[0], util::Globals::blur_brightimage_unit);
    glBindVertexArray(quad_VAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Dump depth information from scene fbo to default fbo
    glBindFramebuffer(GL_READ_FRAMEBUFFER, util::Globals::scene_fbo_ms);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, util::Globals::camera.width(), util::Globals::camera.height(),
        0, 0, util::Globals::camera.width(), util::Globals::camera.height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindVertexArray(0);
}

void Postproc_quad::lighting_pass(const Lighting_pass &shader, const Camera &camera, GLuint fbo) const
{
    assert(shader.shader_dir() == "./shader/lighting_pass");
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindVertexArray(quad_VAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.set_uniforms(camera);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Dump depth information from GBuffer fbo to current light_pass fbo
    glBindFramebuffer(GL_READ_FRAMEBUFFER, util::Globals::Gbuffer_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, util::Globals::camera.width(), util::Globals::camera.height(),
        0, 0, util::Globals::camera.width(), util::Globals::camera.height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Postproc_quad::ssao_pass(const SSAO &shader, const SSAO_blur &blur_shader, GLuint ssao_fbo, GLuint ssao_blur_fbo) const
{
    assert(shader.shader_dir() == "./shader/ssao");
    assert(blur_shader.shader_dir() == "./shader/ssao_blur");
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
    glBindVertexArray(quad_VAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    shader.use();
    shader.set_uniforms();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo);
    blur_shader.use();
    blur_shader.set_uniforms();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

void Postproc_quad::run_blur(const Bloom_blur &shader, GLuint &blur_buffer_unit) const
{
    assert(shader.shader_dir() == "./shader/bloom_blur");
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
        glBindFramebuffer(GL_FRAMEBUFFER, util::Globals::pingpong_fbos[horizental]);
        glBindVertexArray(quad_VAO);
        for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
        shader.use();
        // 现在要blur哪个fbo的color attachment
        shader.set_uniforms(i == 0 ? util::Globals::scene_unit[1] : util::Globals::pingpong_colorbuffers_units[!horizental], horizental);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizental = !horizental;
    }
    blur_buffer_unit = util::Globals::pingpong_colorbuffers_units[0];

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

Postproc_quad::Postproc_quad()
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