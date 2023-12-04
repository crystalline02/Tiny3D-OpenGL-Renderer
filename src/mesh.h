#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "material.h"

class Blinn_phong;
class Normal;
class Single_color;
class Depth_cubemap;
class Tangent_normal;
class Cascade_map;
class Camera;
class Direction_light;
class Point_light;
class Light;
class Depth_shader;
class G_buffer;

struct Vertex
{
    glm::vec3 positon;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texture_coords;
};

class Mesh
{
public:
    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const Material &material);
    ~Mesh();
    void draw(const Blinn_phong &shader, const Camera &camera, const glm::mat4 &model, GLuint fbo = 0) const;
    void draw_gbuffer(const G_buffer &shader, const Camera &camera, const glm::mat4 &model, GLuint fbo) const;
    void draw_normals(const Normal &shader, const Camera &camera, const glm::mat4 &model) const;
    void draw_tangent(const Tangent_normal &shader, const Camera &camera, const glm::mat4 &model) const;
    void draw_outline(const Single_color &shader, const Camera &camera, const glm::mat4 &model) const;
    void draw_depthmap(const Depth_shader &depth_shader, const Light &light, const glm::mat4 &model) const;
    void clear();
    inline bool is_blend_mesh() const { return m_material.is_blend_mat(); }
    inline glm::vec3 center() const { return m_center; }
    inline void set_outline(bool isoutline) { m_outlined = isoutline; }
    inline bool is_outline() const { return m_outlined; }
    inline void set_blend(bool isblend) { m_isblend = isblend; }
    inline bool isblend() const { return m_isblend; }
    inline void set_cullface(bool cullface) { m_iscullface = cullface; }
    void switch_mat_type(Mat_type new_type);
private:
    void setup_mesh();
    bool m_outlined;
    std::vector<Vertex> m_vertices;
    glm::vec3 m_center;
    std::vector<unsigned int> m_indices;
    Material m_material;
    unsigned int VAO, VBO, EBO, VAO_normal, VAO_tangent;
    bool m_isblend, m_iscullface;
};
