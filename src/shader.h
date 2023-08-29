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
    void set_skybox(const char* name, const Camera& camera) const;
protected:
    std::string m_dir;
    static GLuint ubo_matrices, ubo_fn, ubo_light_matrices;
    GLuint program_id;
};

class Blinn_phong: public Shader
{
public:
    Blinn_phong();
    void set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const;
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

class Blur: public Shader
{
public:
    void set_uniforms(GLuint image_unit, bool horizental) const;
    static Blur* get_instance();
private:
    Blur();
    static Blur* instance;
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
