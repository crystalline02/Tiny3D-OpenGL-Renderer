#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <vector>
#include <array>

#include "texture.h"

enum class Light_type{ POINT, SUN, SPOT };

namespace Shader
{
    class Shader;
}

class Camera;
class Model;
struct Texture;

class Light
{
public:
    Light(const glm::vec3& positon = glm::vec3(1.5f, 1.5f, 1.5f), 
        const glm::vec3& color = glm::vec3(.5f), 
        float intensity = 10.f, 
        float diffuse = 1.f, 
        float specular = 1.f,
        float ambient = .005f);
    inline GLuint depth_FBO() const { return m_depth_fbo; }
    inline float& intensity_ptr() { return m_intensity; } 
    inline float& colorPtr() { return m_color.x; }
    inline glm::vec3 color() const { return m_color; }
    inline float diffuse() const { return m_diffuse; }
    inline float specular() const { return m_specular; }
    inline float ambient() const { return m_ambient; }
    inline float intensity() const { return m_intensity; }
    inline float near() const { return m_near; }
    inline float far() const { return m_far; }
    inline glm::vec3 position() const { return m_position; }
    inline void set_positon(const glm::vec3& position) { m_position = position; }
    inline unsigned int index() const { return m_index; }
    virtual void set_uniforms(const char* name, GLuint program) const;
    virtual Light_type type() const = 0;
    virtual void resize_depthmap(int shadow_size) const = 0;
    static std::vector<Light*>& get_lights();  // 静态成员函数只能访问静态成员变量，不能访问非静态的成员变量
    static unsigned int& point_lights_count();
    static unsigned int& spot_lights_count();
    static unsigned int& direction_lights_count();
    static unsigned int lights_count();
public:
    float m_intensity, m_diffuse, m_specular, m_ambient, m_near, m_far;
    glm::vec3 m_color, m_position;
    GLuint m_depth_fbo;
    Texture m_depth_texture;
    unsigned int m_index;
};

class Point_light: public Light
{
public:
    Point_light(const glm::vec3& positon = glm::vec3(1.5f, 1.5f, 1.5f),
        const glm::vec3& color = glm::vec3(1.f, 1.f, 1.f),
        float intensity = 10.f,
        float diffuse = 1.f,
        float specular = 1.f,
        bool flag = false);
    inline float constant() const { return kc; }
    inline float linear() const { return kl; }
    inline float quadratic() const { return kq; }
    void set_uniforms(const char* name, GLuint program) const override;
    virtual std::array<glm::mat4, 6> light_space_mat() const;
    virtual void resize_depthmap(int shadow_size) const override;
    inline Light_type type() const override { return Light_type::POINT; }
protected:
    float kc, kl, kq;
};

class Direction_light: public Light
{
public:
    Direction_light(const glm::vec3& direction = glm::vec3(0.f, -1.f, 0.f),
        const glm::vec3& position = glm::vec3(1.5f, 1.5f, 1.5f),
        const glm::vec3& color = glm::vec3(.5f),
        float intensity = 10.f, 
        float diffuse = 1.f, 
        float specular = 1.f);
    void set_uniforms(const char* name, GLuint program) const override;
    virtual void resize_depthmap(int shadow_size) const override;
    inline glm::vec3 direction() const { return m_direction; }
    inline Light_type type() const override { return Light_type::SUN; }
    std::vector<glm::mat4> light_space_mat(const Camera& camera) const;
protected:
    glm::vec3 m_direction;
};

class Spot_light: public Point_light
{
public:
    Spot_light(const glm::vec3& direction = glm::vec3(0.f, -1.f, 0.f),
        const glm::vec3& position = glm::vec3(1.5f, 1.5f, 1.5f),
        const glm::vec3& color = glm::vec3(.5f),
        float intensity = 10.f, 
        float diffuse = 1.f, 
        float specular = 1.f);
    void set_uniforms(const char* name, GLuint program) const override;
    inline virtual std::array<glm::mat4, 6> light_space_mat() const { return Point_light::light_space_mat(); }
    inline glm::vec3 direction() const { return m_dir; }
    inline void set_cutoff(float cutoff) { m_innner = glm::radians(cutoff); }
    inline void set_outer_cutoff(float outer_cutoff) { m_outer = glm::radians(outer_cutoff); }
    inline float cutoff() const { return m_innner; }
    inline float outer_cutoff() const { return m_outer; }
    inline Light_type type() const override { return Light_type::SPOT; }
protected:
    float m_innner, m_outer;
    glm::vec3 m_dir;
};

class Light_vertices
{
public:
    static Light_vertices* getInstance();
    inline GLuint circle_VAO() const { return m_circle_VAO; }
    inline GLuint bar_VAO() const { return m_bar_VAO; }
    inline GLuint radiant_VAO() const { return m_radiant_VAO; }
    inline GLuint size_vertices_circle() const { return m_size_vertices_circle; }
    inline GLuint size_verticies_bar() const { return m_size_verticies_bar; }
    inline GLuint size_vertices_radiant() const { return m_size_vertices_radiant; }
    void forward(const Shader::Shader& shader, const Camera& camera);
private:
    Light_vertices();
    ~Light_vertices();
    void bind_vbo_2_vao(GLint VAO, GLint VBO) const;
    glm::mat4 get_bar_dir_matrix(glm::vec3 direction, glm::vec3 postion);
    float *m_vertices_circle, *m_verticies_bar, *m_vertices_radiant;
    GLuint m_size_vertices_circle, m_size_verticies_bar, m_size_vertices_radiant;
    GLuint m_circle_VBO, m_bar_VBO, m_radiant_VBO, m_circle_VAO, m_bar_VAO, m_radiant_VAO;
    static Light_vertices* singleton;
};
