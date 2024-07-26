#pragma once
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

class Material;
enum class Mat_type;

namespace Shader
{
    class BlinnPhong;
    class Normal;
    class SingleColor;
    class DepthCubemap;
    class TangentNormal;
    class CascadeMap;
    class DepthShader;
    class ObjectShader;
    class GBuffer;
    class TransparentWBPBR;
}

class Point_light;
class Direction_light;
class Light;
class Camera;

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
    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, Material* material);
    ~Mesh();
    void forwardPass(const Shader::ObjectShader &shader, const Camera &camera, const glm::mat4 &model, GLuint fbo) const;
    void gbufferPass(const Shader::GBuffer &shader, const Camera &camera, const glm::mat4 &model, GLuint fbo) const;
    void depthmapPass(const Shader::DepthShader &shader, const Light &light, const glm::mat4 &model) const;
    void transparentPass(const Shader::TransparentWBPBR& shader, const Camera& camera, const glm::mat4& model, GLuint fbo) const;
    void forwardNormals(const Shader::Normal &shader, const Camera &camera, const glm::mat4 &model) const;
    void forwardTangent(const Shader::TangentNormal &shader, const Camera &camera, const glm::mat4 &model) const;
    void draw_outline(const Shader::SingleColor &shader, const Camera &camera, const glm::mat4 &model) const;
    void clear();
    bool isTransparent() const;
    inline glm::vec3 center() const { return m_center; }
    inline void set_outline(bool isoutline) { m_outlined = isoutline; }
    inline bool is_outline() const { return m_outlined; }
    inline void set_blend(bool isblend) { m_isblend = isblend; }
    inline bool isAlphablend() const { return m_isblend; }
    inline void set_cullface(bool cullface) { m_iscullface = cullface; }
    void switch_mat_type(Mat_type new_type);
private:
    void setup_mesh();
    bool m_outlined;
    std::vector<Vertex> m_vertices;
    glm::vec3 m_center;
    std::vector<unsigned int> m_indices;
    Material* m_material;
    unsigned int VAO, VBO, EBO, VAO_normal, VAO_tangent;
    bool m_isblend, m_iscullface;
};
