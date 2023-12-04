#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Camera;
class Material;
class Light;

class Shader
{
public:
    Shader(std::string dir_path);
    inline GLuint program() const { return program_id; }
    inline void use() const { glUseProgram(program_id); }
    inline std::string shader_dir() const { return m_dir; }
    static void update_uniform_blocks(const Camera& camera);
    void set_single_color(const glm::vec3& color) const;
    void set_model(const glm::mat4& model) const;
    void set_viewpos(const glm::vec3& view_pos) const;
    void set_globals() const;
    void set_normal_mat(const glm::mat4& model) const;
    void set_material(const char* name, const Material& material) const;
protected:
    std::string m_dir;
    static GLuint ubo_matrices, ubo_fn, ubo_light_matrices;
    GLuint program_id;
};

class Object_shader: public Shader
{
public:
    Object_shader(std::string dir_path);
    virtual void set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const = 0;
};

class Blinn_phong: public Object_shader
{
public:
    Blinn_phong();
    void set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const override;
};

class PBR: public Object_shader
{
public:
    PBR();
    void set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const override;
};

class Single_color: public Shader
{
public:
    Single_color();
    void set_uniforms(const glm::mat4& model, const glm::vec3& color) const;
};

class Normal: public Shader
{
public:
    Normal();
    void set_uniforms(const Camera& camera, const glm::mat4& model) const;
};

class Tangent_normal: public Shader
{
public:
    Tangent_normal();
    void set_uniforms(const Camera& camera, const glm::mat4& model) const;
};

class Depth_shader: public Shader
{
public:
    Depth_shader(std::string dir_path);
    virtual void set_uniforms(const glm::mat4& model, const Light& light) const = 0;
};

class Cascade_map: public Depth_shader
{
public:
    Cascade_map();
    void set_uniforms(const glm::mat4& model, const Light& light) const override;
};

class Depth_cubemap: public Depth_shader
{
public:
    Depth_cubemap();
    void set_uniforms(const glm::mat4& model, const Light& light) const override;
};

class Sky_cube: public Shader
{
public:
    Sky_cube();
    void set_uniforms(const Camera& camera, float intensity, GLuint texture_unit) const;
};

class Bloom_blur: public Shader
{
public:
    void set_uniforms(GLuint image_unit, bool horizental) const;
    static Bloom_blur* get_instance();
private:
    Bloom_blur();
    static Bloom_blur* instance;
};

class SSAO_blur: public Shader
{
public:
    static SSAO_blur* get_instance();
    void set_uniforms() const;
private:
    SSAO_blur();
    static SSAO_blur* instance;
};

class Post_proc: public Shader
{
public:
    void set_uniforms(GLuint image_unit, GLuint blured_image_unit) const;
    static Post_proc* get_instance();
private:
    Post_proc();
    static Post_proc* instance;
};

class G_buffer: public Shader
{
public:
    void set_uniforms(const Material &material, const Camera &camera, const glm::mat4 &model) const;
    static G_buffer* get_instance();
private:
    G_buffer();
    static G_buffer* instance;
};

class SSAO: public Shader
{
public:
    void set_uniforms() const;
    static SSAO* get_instance();
private:
    SSAO();
    static SSAO* instance;
};

class Lighting_pass: public Shader
{
public:
    void set_uniforms(const Camera &camera) const;
    static Lighting_pass* get_instance();
private:
    Lighting_pass();
    static Lighting_pass* instance;
};