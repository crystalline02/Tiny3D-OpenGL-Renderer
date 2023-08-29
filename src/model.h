#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"

class Camera;
class Blinn_phong;
class Normal;
class Single_color;
class Cascade_map;
class Depth_shader;
class Cascade_shader;
class Tangent_normal;
class Depth_cubemap;
class Material;
class Light;

class Model
{
public:
    Model(const char *path, const glm::mat4& model = glm::mat4(1.f));
    void draw(const Blinn_phong& model_shader, const Camera& camera, GLuint fbo = 0) const;
    void draw_normals(const Normal& normal_shader, const Camera& camera) const;
    void draw_tangent(const Tangent_normal& tangent_shader, const Camera& camera) const;
    void draw_outline(const Single_color& single_color, const Camera& camera) const;
    void draw_depthmaps(const Cascade_map& depth_shader, const Depth_cubemap& depth_cube_shader) const;
    void switch_model(const std::string new_path);
    inline void set_model(glm::mat4 model) { m_model_mat = model; }
    inline static GLuint num_textures() { return Model::loaded_textures.size(); }
    inline static std::vector<std::string> all_models() { return model_dirs; }
    inline static unsigned int seleted_model() { return seleted; }
    inline static void set_selected(unsigned int id) { seleted = id; }
    inline static void add_texture(const char* path, unsigned int unit) { Model::loaded_textures[path] = unit; }
    void set_blend(bool isblend);
    void set_outline(bool isoutline);
    void set_cullface(bool iscull);
private:
    void load_model(std::string path);
    void process_node(aiNode* node, const aiScene* scene);
    Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<unsigned int> retrive_texture_units(aiMaterial* materia, aiTextureType type);
    Material process_material(aiMaterial* material);
    std::vector<Mesh> m_meshes;
    std::string directory;
    glm::mat4 m_model_mat;

    static std::unordered_map<std::string, unsigned int> loaded_textures;  // 记录所有的Model一共加载了多少个贴图，每个贴图的路径和texture_unit都被记载
    static std::vector<std::string> model_dirs;
    static unsigned int seleted;
};