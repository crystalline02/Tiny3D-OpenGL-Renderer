#include "postproc.h"
#include "globals.h"
#include "shader.h"

Postproc_quad* Postproc_quad::instance = nullptr;

Postproc_quad *Postproc_quad::get_instance()
{
    if(!instance) return new Postproc_quad();
    else return instance;
}

void Postproc_quad::draw(const Post_proc& shader)
{
    assert(shader.shader_dir() == "./shader/post_quad");
    if(util::Globals::bloom)
        run_blur(*Blur::get_instance(), util::Globals::blur_brightimage_unit);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shader.use();
    shader.set_uniforms(util::Globals::scene_unit[0], util::Globals::blur_brightimage_unit);
    glBindVertexArray(quad_VAO);
    for(int i = 0; i < 2; ++i) glEnableVertexAttribArray(i);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Postproc_quad::run_blur(const Blur &shader, GLuint &blur_image_unit)
{
    assert(shader.shader_dir() == "./shader/blur");
    if(!util::Globals::bloom) 
    {
        blur_image_unit = -1;
        return;
    }
    bool horizental = true;
    int amount = 16;
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
    blur_image_unit = util::Globals::pingpong_colorbuffers_units[0];

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