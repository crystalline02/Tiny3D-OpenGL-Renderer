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
    class Blinn_phong;
    class Normal;
    class Single_color;
    class Cascade_map;
    class Depth_shader;
    class Cascade_shader;
    class Object_shader;
    class Tangent_normal;
    class Depth_cubemap;
    class G_buffer;
    class Lighting_pass;
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
    void draw(const Shader::Object_shader &shader, const Camera& camera, GLuint fbo = 0) const;
    void gbuffer_pass(const Shader::G_buffer &shader, const Camera& camera, GLuint fbo);
    void forward_tranparent(const Shader::Object_shader &shader, const Camera& camera, GLuint fbo);
    void draw_normals(const Shader::Normal &shader, const Camera &camera) const;
    void draw_tangent(const Shader::Tangent_normal &shader, const Camera &camera) const;
    void draw_outline(const Shader::Single_color &shader, const Camera &camera) const;
    void draw_depthmaps(const Shader::Cascade_map &shader_cascade, const Shader::Depth_cubemap &shader_depthcube) const;
    void reload(const std::string new_path);
    static void log_texture_info();
    static void switch_model(Model &model);
    static Texture get_texture(const char* texname);
    static GLuint fatch_new_texunit();
    static void add_texture(const char* name, const Texture& texture);
    static void rm_texture(const Texture& texture);
    inline static void set_selected(GLuint ind) { new_selected_ind = ind; }
    inline void set_model(glm::mat4 model) { m_model_mat = model; }
    inline static std::vector<std::string> all_models() { return model_dirs; }
    inline static unsigned int seleted_model() { return selected_ind; }
    void switch_mat_type(Mat_type new_type);
    void set_blend(bool isblend);
    void set_outline(bool isoutline);
    void set_cullface(bool iscull);
private:
    void load_model(std::string path);
    void clear_model();
    void process_node(aiNode* node, const aiScene* scene);
    Mesh* process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> retrive_textures(aiMaterial* material, aiTextureType type);
    Material* process_material(aiMaterial* material);
private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;  // shared_ptr
    std::map<float, const Mesh*> m_blend_meshes_temp;
    std::string directory;
    glm::mat4 m_model_mat;

    static std::unordered_map<std::string, Texture> loaded_textures;  // 记录所有的Model一共加载了多少个贴图，每个贴图的路径和texture_unit都被记载
    static std::vector<std::string> model_dirs;
    static GLuint selected_ind, new_selected_ind;
};