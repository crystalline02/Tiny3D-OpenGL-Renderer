#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Shader
{
    class BlinnPhong;
    class Normal;
    class SingleColor;
    class CascadeMap;
    class DepthShader;
    class Cascade_shader;
    class ObjectShader;
    class TangentNormal;
    class DepthCubemap;
    class GBuffer;
    class LightingPass;
    class TransparentWBPBR;
}
class Camera;
class Material;
class Light;
class Mesh;
struct Texture;
enum class Mat_type;

class Model
{
public:
    Model(const char *path, const glm::mat4& model = glm::mat4(1.f));
    ~Model();
    void foward(const Shader::ObjectShader &shader, const Camera& camera, GLuint fbo = 0) const;
    void gbufferPass(const Shader::GBuffer &shader, const Camera& camera, GLuint fbo);
    void transparentPass(const Shader::TransparentWBPBR &shader, const Camera& camera, GLuint fbo);
    void compositePass() const;
    void drawNormals(const Shader::Normal &shader, const Camera &camera) const;
    void drawTangent(const Shader::TangentNormal &shader, const Camera &camera) const;
    void drawOutline(const Shader::SingleColor &shader, const Camera &camera) const;
    void depthmapPass(const Shader::CascadeMap &shader_cascade, const Shader::DepthCubemap &shader_depthcube) const;
    void reload(const std::string new_path);
    static void log_texture_info();
    static void switch_model(Model &model);
    static Texture getTexture(const char* texname);
    static GLuint fetchNewTexunit();
    static void add_texture(const char* name, const Texture& texture);
    static void rmTexture(const Texture& texture);
    inline static void set_selected(GLuint ind) { new_selected_ind = ind; }
    inline void set_model(glm::mat4 model) { m_model_mat = model; }
    inline static std::vector<std::string> all_models() { return model_dirs; }
    inline static unsigned int seleted_model() { return selected_ind; }
    void switch_mat_type(Mat_type new_type);
    void setBlend(bool isblend);
    void set_outline(bool isoutline);
    void setCullface(bool iscull);
private:
    void load_model(std::string path);
    void clear_model();
    void process_node(aiNode* node, const aiScene* scene);
    Mesh* process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> retrive_textures(aiMaterial* material, aiTextureType type);
    Material* process_material(aiMaterial* material);
private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;  // shared_ptr
    std::vector<std::shared_ptr<Mesh>> m_transparentMeshes;
    std::string directory;
    glm::mat4 m_model_mat;

    static std::unordered_map<std::string, Texture> loaded_textures;  // 记录所有的Model一共加载了多少个贴图，每个贴图的路径和texture_unit都被记载
    static std::vector<std::string> model_dirs;
    static GLuint selected_ind, new_selected_ind;
};