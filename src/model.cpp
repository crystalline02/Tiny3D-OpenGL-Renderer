#include "model.h"
#include "mesh.h"
#include "camera.h"
#include "shader.h"
#include "material.h"
#include "globals.h"
#include "light.h"

#include <glm/glm.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <set>


std::unordered_map<std::string, Texture> Model::loaded_textures;
std::vector<std::string> Model::model_dirs = {};
GLuint Model::selected_ind = 1;
GLuint Model::new_selected_ind = 1;

Model::Model(const char *path, const glm::mat4& model): 
m_model_mat(model)
{
    std::cout << "Loading model from " << path << "......\n";
    auto start = std::chrono::high_resolution_clock::now();
    load_model(std::string(path));
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Model loaded, time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms.\n";
    if(model_dirs.empty())
    {
        for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator("./model"))
        {
            std::string cur_dir_name = entry.path().string();
            if(std::filesystem::is_directory(cur_dir_name))
            {
                size_t pos = 0;
                while((pos = cur_dir_name.find("\\", pos)) != std::string::npos)
                    cur_dir_name.replace(pos, 1, "/");
                model_dirs.push_back(cur_dir_name);
            }
        }
    }
}

Model::~Model()
{
    clear_model();
}

void Model::log_texture_info()
{
    int col_width = 60;
    std::vector<std::string> headers = {"Texname", "Texunit", "Texbuffer"};
    util::print_row(headers, col_width);
    std::cout << std::setfill('-') << std::setw(headers.size() * (col_width + 1) + 1) << "" << std::endl;
    std::cout << std::setfill(' ');
    for(std::unordered_map<std::string, Texture>::iterator it = loaded_textures.begin(); it != loaded_textures.end(); ++it)
        util::print_row({std::string(it->second.texname), 
            std::to_string(it->second.texunit), 
            std::to_string(it->second.texbuffer)}, 
                col_width);
    std::cout << std::endl;
}

void Model::draw_normals(const Shader::Normal& normal_shader, const Camera& camera) const
{
    for(const std::shared_ptr<Mesh>& mesh: m_meshes) mesh->draw_normals(normal_shader, camera, m_model_mat);
}

void Model::draw_tangent(const Shader::Tangent_normal &tangent_shader, const Camera &camera) const
{
    for(const std::shared_ptr<Mesh>& mesh: m_meshes) mesh->draw_tangent(tangent_shader, camera, m_model_mat);
}

void Model::load_model(std::string path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if(!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        std::cout << "Assimp error:" << importer.GetErrorString() << std::endl;
        exit(1);
    }
    directory = path.substr(0, path.find_last_of("/"));
    process_node(scene->mRootNode, scene);
}

void Model::clear_model()
{
    // 这里的clear并没有clear texture buffer，后面可以加上，可能需要改动很多代码
    m_meshes.clear();
    m_blend_meshes_temp.clear();
    directory.clear();
}

void Model::rm_texture(const Texture& texture)
{
    auto it = loaded_textures.find(texture.texname);
    if(it == loaded_textures.end()) return;
    
    loaded_textures.erase(it);
    glActiveTexture(GL_TEXTURE0 + texture.texunit);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texture.texbuffer);
}

void Model::process_node(aiNode* node, const aiScene* scene)
{
    for(int i = 0; i < node->mNumMeshes; ++i)
        m_meshes.push_back(std::make_unique<Mesh>(*process_mesh(scene->mMeshes[node->mMeshes[i]], scene)));
    for(int i = 0; i < node->mNumChildren; ++i)
        process_node(node->mChildren[i], scene);
}

Material* Model::process_material(aiMaterial* material)
{
    std::vector<Texture> diffuse_textures, specular_textures, ambient_textures, 
        opacity_textures, normal_textures, displacement_textures, metalic_textures, roughness_textures;
    diffuse_textures = retrive_textures(material, aiTextureType_DIFFUSE);
    specular_textures = retrive_textures(material, aiTextureType_SPECULAR);
    ambient_textures = retrive_textures(material, aiTextureType_AMBIENT);
    metalic_textures = retrive_textures(material, aiTextureType_EMISSIVE);  // ASSIMP cannot load `map_refl`, so I use map_Ke temporally.
    roughness_textures = retrive_textures(material, aiTextureType_SHININESS);
    opacity_textures = retrive_textures(material, aiTextureType_OPACITY);
    normal_textures = retrive_textures(material, aiTextureType_HEIGHT);
    displacement_textures = retrive_textures(material, aiTextureType_DISPLACEMENT);

    // illumintaion mode
    int illum;
    material->Get(AI_MATKEY_SHADING_MODEL, illum);
    
    Material* ret_mat = new Material(diffuse_textures,
        specular_textures, 
        roughness_textures,
        ambient_textures,
        opacity_textures,
        normal_textures,
        displacement_textures,
        metalic_textures,
        util::Globals::pbr_mat ? Mat_type::PBR : Mat_type::Blinn_Phong);
    
    if(diffuse_textures.empty())
    {
        aiColor3D color;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        ret_mat->set_diffuse_color({color.r, color.g, color.b});
    }
    if(specular_textures.empty())
    {
        aiColor3D color;
        material->Get(AI_MATKEY_COLOR_SPECULAR, color);
        ret_mat->set_specular_color({color.r, color.g, color.b});
    }

    // roughness / shiness
    {
        float shiness;
        material->Get(AI_MATKEY_SHININESS, shiness);
        float roughness = glm::max(0.f, glm::min(shiness, 1000.f));
        roughness = 1.f - glm::sqrt(roughness / 1000.f);
        ret_mat->set_shinness(shiness);
        ret_mat->set_roughness(roughness);
    }

    if(ambient_textures.empty())
    {
        aiColor3D ambient;
        material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        ret_mat->set_ambient_color({ambient.r, ambient.g, ambient.b});
    }
    if(opacity_textures.empty())
    {
        float opacity;
        material->Get(AI_MATKEY_OPACITY, opacity);
        ret_mat->set_opacity(opacity);
    }
    if(metalic_textures.empty())
    {
        float metalic;
        if(illum == 2)
        {  
            // Do reflection
            aiColor3D metalic_color;
            material->Get(AI_MATKEY_COLOR_AMBIENT, metalic_color);
            metalic = (metalic_color.r + metalic_color.g + metalic_color.b) / 3.f;
        }
        else
        {
            // No reflection
            metalic = 0.f;
        }
        ret_mat->set_metalic(metalic);
    }
    return ret_mat;
}

std::vector<Texture> Model::retrive_textures(aiMaterial* material, aiTextureType type)
{
    std::vector<Texture> textures;
    for(int i = 0; i < material->GetTextureCount(type); ++i)
    {
        aiString tex_string;
        material->GetTexture(type, i, &tex_string);
        std::string texture_path = directory + "/" + tex_string.C_Str();
        Texture texture;
        util::create_texture(texture_path.c_str(), type == aiTextureType_DIFFUSE, texture);
        textures.emplace_back(texture);
    }
    return textures;
}

Mesh* Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    for(int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;
        vertex.positon = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        glm::vec3 tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
        vertex.tangent = glm::normalize(glm::dot(tangent, vertex.normal) * vertex.normal - tangent);
        if(mesh->mTextureCoords[0]) // 第0套mTextureCoords有值，那就用第0套
            vertex.texture_coords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};  // [0][i]表示用第0套的第i个顶点的纹理坐标
        else
            vertex.texture_coords = {0.f, 0.f};
        vertices.push_back(vertex);
    }

    for(int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for(int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    Material* material = nullptr;
    if(mesh->mMaterialIndex >=0)
        material = process_material(scene->mMaterials[mesh->mMaterialIndex]);
    else
        material = new Material();

    return new Mesh(vertices, indices, material);
}

Texture Model::get_texture(const char* texname)
{
    std::unordered_map<std::string, Texture>::iterator it = loaded_textures.find(texname);
    if(it == loaded_textures.end()) return Texture("Null texture", 0, 0);
    else return it->second;
}

GLuint Model::fatch_new_texunit()
{
    if(loaded_textures.empty()) return 0;
    
    std::set<GLuint> ordered_unit;
    for(auto it = loaded_textures.begin(); it != loaded_textures.end(); ++it)
        ordered_unit.insert(it->second.texunit);
    GLuint texunit = 0;
    for(std::set<GLuint>::iterator it = ordered_unit.begin(); it != ordered_unit.end(); ++it)
    {
        if(texunit == *it) ++texunit;
        else break;
    }
    return texunit;
}

void Model::add_texture(const char* name, const Texture& texture)
{
    if(loaded_textures.find(name) != loaded_textures.end())
       std::cout << "Warnning: texture of " << name << " is replaced\n";
    loaded_textures[name] = texture;
}

void Model::switch_model(Model &model)
{
    if(selected_ind == new_selected_ind) return;
    selected_ind = new_selected_ind;
    for(const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(model_dirs[selected_ind]))
    {
        std::string name = entry.path().string();
        if(!std::filesystem::is_directory(name))
            if(name.substr(name.find_last_of("."), name.size() - name.find_last_of(".")) == ".obj")
                model.reload(name.replace(name.find("\\", 0), 1, "/"));
    }
}

void Model::switch_mat_type(Mat_type new_type)
{
    for(std::shared_ptr<Mesh>& mesh : m_meshes)
        mesh->switch_mat_type(new_type);
}

void Model::set_blend(bool isblend)
{
    for(std::shared_ptr<Mesh>& mesh: m_meshes) mesh->set_blend(isblend);
}

void Model::set_cullface(bool iscull)
{
    for(std::shared_ptr<Mesh>& mesh: m_meshes) mesh->set_cullface(iscull);
}

void Model::draw(const Shader::Object_shader& model_shader, const Camera& camera, GLuint fbo) const
{
    // This is not a good ideal to solve blending problem, but temporarily works
    std::map<float, const Mesh*> blend_mesh;
    for(const std::shared_ptr<Mesh>& mesh: m_meshes)
    {
        if(!mesh->is_blend_mesh())
        {
            mesh->draw(model_shader, camera, m_model_mat, fbo);
        }
        else 
        {
            blend_mesh[glm::length(glm::vec3(m_model_mat * glm::vec4(mesh->center(), 1.f)) - camera.position())] = mesh.get();
        }
    }
    for(std::map<float, const Mesh*>::reverse_iterator it = blend_mesh.rbegin(); it != blend_mesh.rend() ; ++it)
    {
        it->second->draw(model_shader, camera, m_model_mat, fbo);
    }
    blend_mesh.clear();
}

void Model::gbuffer_pass(const Shader::G_buffer &shader, const Camera &camera, GLuint fbo)
{
    for(const std::shared_ptr<Mesh>& mesh: m_meshes)
    {
        if(mesh->is_blend_mesh())
            m_blend_meshes_temp[glm::length(glm::vec3(m_model_mat * glm::vec4(mesh->center(), 1.f)) - camera.position())] = mesh.get();
        else 
            mesh->draw_gbuffer(shader, camera, m_model_mat, fbo);
    }
}

void Model::forward_tranparent(const Shader::Object_shader &shader, const Camera &camera, GLuint fbo)
{
    for(std::map<float, const Mesh*>::const_reverse_iterator it = m_blend_meshes_temp.rbegin(); it != m_blend_meshes_temp.rend(); ++it)
        it->second->draw(shader, camera, m_model_mat, fbo);
    // 别忘了clear这个map容器，要不然这个容器越来越大，程序越来越卡...
    m_blend_meshes_temp.clear();
}

void Model::reload(const std::string new_path)
{
    clear_model();
    std::cout << "Loading model from " << new_path << "......\n";
    auto start = std::chrono::high_resolution_clock::now();
    load_model(std::string(new_path));
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Model loaded, time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms.\n";
}

void Model::draw_outline(const Shader::Single_color& single_color, const Camera& camera) const
{
    for(const std::shared_ptr<Mesh>& mesh: m_meshes) if(mesh->is_outline()) mesh->draw_outline(single_color, camera, glm::scale(m_model_mat, glm::vec3(1.008f)));
    glClear(GL_STENCIL_BUFFER_BIT);
}

void Model::set_outline(bool isoutline)
{
    for(std::shared_ptr<Mesh>& mesh: m_meshes)
        mesh->set_outline(isoutline);
}

void Model::draw_depthmaps(const Shader::Cascade_map& cascade_shader, const Shader::Depth_cubemap& depth_cube_shader) const
{
    assert(cascade_shader.shader_name() == "./shader/cascade_map");
    assert(depth_cube_shader.shader_name() == "./shader/depth_cubemap");
    std::vector<Light*> all_lights = Light::get_lights();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    for(Light* light: all_lights)
    {
        assert(light->depth_FBO() != -1);
        glBindFramebuffer(GL_FRAMEBUFFER, light->depth_FBO());
        glClear(GL_DEPTH_BUFFER_BIT);
        util::checkFrameBufferInComplete("DepthFbo");
        if(light->type() == Light_type::SUN)
        {
            // shadow pass for directional light
            glViewport(0, 0, util::Globals::cascade_size, util::Globals::cascade_size);
            for(const std::shared_ptr<Mesh>& mesh: m_meshes) mesh->draw_depthmap(cascade_shader, *light, m_model_mat);
        }
        else
        {
            // shadow pass for point light
            glViewport(0, 0, util::Globals::cubemap_size, util::Globals::cubemap_size);
            for(const std::shared_ptr<Mesh>& mesh: m_meshes) mesh->draw_depthmap(depth_cube_shader, *light, m_model_mat);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, util::Globals::camera.width(), util::Globals::camera.height());
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}