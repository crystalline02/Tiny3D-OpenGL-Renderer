#include "model.h"
#include "mesh.h"
#include "camera.h"
#include "shader.h"
#include "material.h"
#include "globals.h"
#include "light.h"

#include <chrono>
#include <map>
#include <iostream>
#include <filesystem>

std::unordered_map<std::string, unsigned int> Model::loaded_textures;
std::vector<std::string> Model::model_dirs = {};
unsigned int Model::seleted = 0;

Model::Model(const char *path, const glm::mat4& model): 
m_model_mat(model)
{
    std::cout << "Loading model from" << path << "......\n";
    auto start = std::chrono::high_resolution_clock::now();
    load_model(std::string(path));
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Model loaded, time:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms.\n";
    if(model_dirs.empty())
    {
        for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator("./model"))
            if(std::filesystem::is_directory(entry.path()))
                model_dirs.push_back(entry.path().string());
    }
}

void Model::draw_normals(const Normal& normal_shader, const Camera& camera) const
{
    for(const Mesh& mesh: m_meshes) mesh.draw_normals(normal_shader, camera, m_model_mat);
}

void Model::draw_tangent(const Tangent_normal &tangent_shader, const Camera &camera) const
{
    for(const Mesh& mesh: m_meshes) mesh.draw_tangent(tangent_shader, camera, m_model_mat);
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

void Model::process_node(aiNode* node, const aiScene* scene)
{
    for(int i = 0; i < node->mNumMeshes; ++i)
        m_meshes.push_back(process_mesh(scene->mMeshes[node->mMeshes[i]], scene));
    for(int i = 0; i < node->mNumChildren; ++i)
        process_node(node->mChildren[i], scene);
}

Material Model::process_material(aiMaterial* material)
{
    std::vector<unsigned int> diffuse_maps_units, specular_maps_units, ambient_maps_units, opacity_maps_units, normal_maps_units, displacement_maps_units;
    diffuse_maps_units = retrive_texture_units(material, aiTextureType_DIFFUSE);
    specular_maps_units = retrive_texture_units(material, aiTextureType_SPECULAR);
    ambient_maps_units = retrive_texture_units(material, aiTextureType_AMBIENT);
    opacity_maps_units = retrive_texture_units(material, aiTextureType_OPACITY);
    normal_maps_units = retrive_texture_units(material, aiTextureType_HEIGHT);
    displacement_maps_units = retrive_texture_units(material, aiTextureType_DISPLACEMENT);
    Material ret_mat(diffuse_maps_units,
        specular_maps_units, 
        ambient_maps_units,
        opacity_maps_units,
        normal_maps_units,
        displacement_maps_units);
    if(diffuse_maps_units.empty())
    {
        aiColor3D color;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        ret_mat.set_diffuse_color({color.r, color.g, color.b});
    }
    if(specular_maps_units.empty())
    {
        aiColor3D color;
        material->Get(AI_MATKEY_COLOR_SPECULAR, color);
        ret_mat.set_specular_color({color.r, color.g, color.b});
    }
    if(ambient_maps_units.empty())
    {
        aiColor3D color;
        material->Get(AI_MATKEY_COLOR_AMBIENT, color);
        ret_mat.set_ambient_color({color.r, color.g, color.b});
    }
    if(opacity_maps_units.empty())
    {
        float opacity;
        material->Get(AI_MATKEY_OPACITY, opacity);
        ret_mat.set_opacity(opacity);
    }
    return ret_mat;
}

std::vector<unsigned int> Model::retrive_texture_units(aiMaterial* material, aiTextureType type)
{
    std::vector<unsigned int> texture_units;
    for(int i = 0; i < material->GetTextureCount(type); ++i)
    {
        aiString tex_string;
        material->GetTexture(type, i, &tex_string);
        std::string texture_path = directory + "/" + tex_string.C_Str();
        std::unordered_map<std::string, unsigned int>::iterator it = loaded_textures.find(texture_path);
        if(it == loaded_textures.end())  // 没有找到相同贴图
        {
            unsigned int texture_unit = loaded_textures.size();
            util::create_texture(texture_unit, 
                texture_path.c_str(), type == aiTextureType_DIFFUSE ? true : false);
            texture_units.push_back(texture_unit);
        }
        else texture_units.push_back(it->second);
    }
    return texture_units;
}

Mesh Model::process_mesh(aiMesh* mesh, const aiScene* scene)
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

    Material material;
    if(mesh->mMaterialIndex >=0)
        material = process_material(scene->mMaterials[mesh->mMaterialIndex]);
    else
        material = Material();

    return Mesh(vertices, indices, material);
}

void Model::set_blend(bool isblend)
{
    for(Mesh& mesh: m_meshes) mesh.set_blend(isblend);
}

void Model::set_cullface(bool iscull)
{
    for(Mesh& mesh: m_meshes) mesh.set_cullface(iscull);
}

void Model::draw(const Blinn_phong& model_shader, const Camera& camera, GLuint fbo) const
{
    // This is not a good ideal to solve blending problem, but temporarily works
    std::map<float,const Mesh*> blend_meshes;
    for(const Mesh& mesh: m_meshes)
    {
        if(!mesh.is_blend_mesh())
            mesh.draw(model_shader, camera, m_model_mat, fbo);
        else  
            blend_meshes[glm::length(glm::vec3(m_model_mat * glm::vec4(mesh.center(), 1.f)) - camera.position())] = &mesh;
    }
    for(std::map<float, const Mesh*>::reverse_iterator it = blend_meshes.rbegin(); it != blend_meshes.rend() ; ++it)
        it->second->draw(model_shader, camera, m_model_mat);
}

void Model::switch_model(const std::string new_path)
{
    directory = new_path.substr(0, new_path.find_last_of("/"));
}

void Model::draw_outline(const Single_color& single_color, const Camera& camera) const
{
    for(const Mesh& mesh: m_meshes) if(mesh.is_outline()) mesh.draw_outline(single_color, camera, glm::scale(m_model_mat, glm::vec3(1.008f)));
    glClear(GL_STENCIL_BUFFER_BIT);
}

void Model::set_outline(bool isoutline)
{
    for(Mesh& mesh: m_meshes)
        mesh.set_outline(isoutline);
}

void Model::draw_depthmaps(const Cascade_map& cascade_shader, const Depth_cubemap& depth_cube_shader) const
{
    assert(cascade_shader.shader_dir() == "./shader/cascade_map");
    assert(depth_cube_shader.shader_dir() == "./shader/depth_cubemap");
    std::vector<Light*> all_lights = Light::get_lights();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    for(Light* light: all_lights)
    {
        assert(light->depth_FBO() != -1);
        glBindFramebuffer(GL_FRAMEBUFFER, light->depth_FBO());
        glClear(GL_DEPTH_BUFFER_BIT);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Depth framebuffer incompelete!\n";
            exit(1);
        }
        if(light->type() == Light_type::SUN)
        {
            glViewport(0, 0, util::Globals::cascade_size, util::Globals::cascade_size);
            for(const Mesh& mesh: m_meshes) mesh.draw_depthmap(cascade_shader, *light, m_model_mat);
        }
        else
        {
            glViewport(0, 0, util::Globals::cubemap_size, util::Globals::cubemap_size);
            for(const Mesh& mesh: m_meshes) mesh.draw_depthmap(depth_cube_shader, *light, m_model_mat);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, util::Globals::camera.width(), util::Globals::camera.height());
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}