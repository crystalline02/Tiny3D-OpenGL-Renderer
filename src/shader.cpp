#include "shader.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "material.h"
#include "light.h"
#include "camera.h"
#include "globals.h"
#include "model.h"
#include "skybox.h"

GLuint Shader::Shader::ubo_matrices = -1, Shader::Shader::ubo_fn = -1, Shader::Shader::ubo_light_matrices = -1;

Shader::Shader::Shader(std::string dir_path): m_dir(dir_path)
{
    std::string vertex_shader_path = (dir_path + "/vertex_shader.vs"),
    fragment_shader_path = (dir_path + "/fragment_shader.fs"),
    geometry_shader_path = (dir_path + "/geometry_shader.gs");
    std::ifstream vertex_shader_file, fragment_shader_file, geometry_shader_file;
    std::stringstream vertex_shader_ss, fragment_shader_ss, geometry_shader_ss;
    std::string vectex_shader_str, fragment_shader_str, geometry_shader_str;
    const char* vertex_shader_source, *fragment_shader_source, *geometry_shader_source;
    try
    {
        vertex_shader_file.open(vertex_shader_path.c_str());
        fragment_shader_file.open(fragment_shader_path.c_str());
        if(!vertex_shader_file.is_open())
        { 
            std::cout << vertex_shader_path << " not opened successfully." << std::endl;
            exit(1);
        }
        if(!fragment_shader_file.is_open()) 
        { 
            std::cout << fragment_shader_path << " not opened successfully." << std::endl; 
            exit(1);
        }
        vertex_shader_ss << vertex_shader_file.rdbuf();
        fragment_shader_ss << fragment_shader_file.rdbuf();
        vectex_shader_str = vertex_shader_ss.str();
        fragment_shader_str = fragment_shader_ss.str();
        vertex_shader_file.close();
        fragment_shader_file.close();
    }
    catch(const std::ifstream::failure& e)
    {
        std::cout << "Shader file not read successfully, " << e.what() << std::endl;
    }
    if(std::ifstream(geometry_shader_path).good())
    {
        try
        {
            geometry_shader_file.open(geometry_shader_path);
            if(!geometry_shader_file.is_open())
            {
                std::cout << geometry_shader_path << " not opened successfullly.\n";
                exit(1);
            }
            geometry_shader_ss << geometry_shader_file.rdbuf();
            geometry_shader_str = geometry_shader_ss.str();
            if(geometry_shader_path == "./shader/cascadeMap/geometry_shader.gs")
            {
                std::string invocation_replacement = "invocations = " + std::to_string(util::Globals::cascade_levels.size() + 1);
                geometry_shader_str = geometry_shader_str.replace(geometry_shader_str.find("invocations = 1"), 
                    invocation_replacement.length(), 
                    invocation_replacement);
            }
            geometry_shader_file.close();
        }
        catch(const std::ifstream::failure& e)
        {
            std::cout << "Shader file not read successfully, " << e.what() << std::endl;
        }
    }

    GLint success = GL_FALSE;
    GLchar infolog[512];
    // Compiling vertex shader
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    vertex_shader_source = vectex_shader_str.c_str();
    // Note: 此处不能写成glShaderSource(vertex_shader_id, 1, &(vertex_shader_source_string.c_str()), nullptr);
    // 这是因为，取引用只能对左值取引用，比如&a,&b，不能直接对一个函数的返回值(是右值)取引用
    glShaderSource(vertex_shader_id, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader_id);
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader_id, 512, nullptr, infolog);
        std::cout << "Vertex shader " << vertex_shader_path << " compiling error:\n" << infolog << std::endl;
    }

    // Compiling fragment shader
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    fragment_shader_source = fragment_shader_str.c_str();
    glShaderSource(fragment_shader_id, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader_id);
    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);  // 获得哪个shader的info，fragment_shader_id？获得什么info？GL_COMPILE_STATUS，获得什么结果？&success得知
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader_id, 512, nullptr, infolog);
        std::cout << "Fragment shader " << fragment_shader_path << " compiling error:\n" << infolog << std::endl;
    }

    // Comiling geometry shader
    GLuint geometry_shader_id = -1;
    if(std::ifstream(geometry_shader_path).good())
    {
        geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);
        geometry_shader_source = geometry_shader_str.c_str();
        glShaderSource(geometry_shader_id, 1, &geometry_shader_source, nullptr);
        glCompileShader(geometry_shader_id);
        glGetShaderiv(geometry_shader_id, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(geometry_shader_id, 512, nullptr, infolog);
            std::cout << "Geometry shader:" << geometry_shader_path << " compiling error:\n" << infolog << std::endl;
        }
    }

    programId = glCreateProgram();
    glAttachShader(programId, vertex_shader_id);
    glAttachShader(programId, fragment_shader_id);
    if(geometry_shader_id != -1) glAttachShader(programId, geometry_shader_id);
    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(programId, 512, nullptr, infolog);
        std::cout << dir_path << " shader program link error:" << infolog << std::endl;
        exit(1);
    }
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    if(geometry_shader_id != -1) glDeleteShader(geometry_shader_id);

    // Binding uniform buffer object and corresbonding uniform block to BindingPoint 0
    if(ubo_matrices == -1 || ubo_fn == -1 || ubo_light_matrices == -1)
    {
        glGenBuffers(1, &ubo_matrices);
        glGenBuffers(1, &ubo_fn);
        glGenBuffers(1, &ubo_light_matrices);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_matrices, 0, 2 * sizeof(glm::mat4));  // Bind ubo_matrics to Binding Point 0
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_fn);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(float), nullptr, GL_STATIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, ubo_fn, 0, 2 * sizeof(float));  // Bind ubo_FN to Binding point 1
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_light_matrices);
        glBufferData(GL_UNIFORM_BUFFER, 80 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, ubo_light_matrices, 0, 80 * sizeof(glm::mat4));  // Bind ubo_light_matrices to Binding point 2
        glBindBuffer(GL_UNIFORM_BUFFER, 0);  // Unbind
    }
}

void Shader::Shader::set_material(const char* name, const Material& material) const
{
    material.set_uniforms(name, programId);
}

void Shader::Shader::set_globals() const
{
    util::set_float("near", util::Globals::camera.near(), programId);
    util::set_float("far", util::Globals::camera.far(), programId);
    util::set_float("bias", util::Globals::shadow_bias, programId);
}

void Shader::Shader::setSingleColor(const glm::vec3& color) const
{
    util::set_floats("single_color", color, programId);
}

void Shader::Shader::update_uniform_blocks(const Camera& camera)
{
    // Uniform buffer for ViewProjection matrices 
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera.lookat()));  // Fill buffer for view matrix
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera.projection()));  // Fill buffer for projection matrix

    // Uniform buffeer for far&near floating point numbers
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_fn);
    float n = camera.near(), f = camera.far();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &f);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float), sizeof(float), &n);
    
    // Unifrom
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_light_matrices);
    std::vector<Light*> all_lights = Light::get_lights(); 
    for(Light*& light: all_lights)
    {
        if(light->type() == Light_type::SUN)
        {
            Direction_light* dir_light = static_cast<Direction_light*>(light);  // 父类指针指向子类对象时，强转父类指针为子类指针，已知可以完成转换，故用static cast
            std::vector<glm::mat4> light_matrices = dir_light->light_space_mat(camera);
            assert(light_matrices.size() <= 10);
            glBufferSubData(GL_UNIFORM_BUFFER, dir_light->index() * 10 * sizeof(glm::mat4), light_matrices.size() * sizeof(glm::mat4), light_matrices.data());
        }
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::Shader::set_model(const glm::mat4& model) const
{
    util::set_mat("model", model, programId);
}

void Shader::Shader::set_normal_mat(const glm::mat4& model) const
{
    util::set_mat("normal_mat", glm::transpose(glm::inverse(glm::mat3(model))), programId);  // 要把normal转换到view space下，注意在glm中非常奇怪，矩阵乘法A * B在底层实现实际上是B * A
}

void Shader::Shader::set_viewpos(const glm::vec3& view_pos) const
{
    util::set_floats("view_pos", view_pos, programId);
}

Shader::ObjectShader::ObjectShader(std::string dir_path): Shader(dir_path)
{

}

Shader::BlinnPhong::BlinnPhong(): ObjectShader("./shader/blinnPhong") 
{     

}

void Shader::BlinnPhong::set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const
{
    util::set_mat("model", model, programId);
    util::set_mat("normal_mat", glm::transpose(glm::inverse(glm::mat3(model))), programId);
    util::set_floats("view_pos", camera.position(), programId);
    material.set_uniforms("material", programId);
    util::set_float("bias", util::Globals::shadow_bias, programId);
    std::vector<Light*> all_lights = Light::get_lights();
    for(Light*& light : all_lights)
    {
        switch(light->type())
        {
        case Light_type::POINT:
        {
            light->set_uniforms("point_lights", programId);
            break;
        }
        case Light_type::SPOT:
        {
            light->set_uniforms("spot_lights", programId);
            break;
        }
        case Light_type::SUN:
        {
            light->set_uniforms("direction_lights", programId);
            break;
        }
        defalut:
            break;
        }
    }
    util::set_bool("bloom", util::Globals::bloom, programId);
    util::set_float("threshold", util::Globals::threshold, programId);
    util::set_int("point_lights_count", Light::point_lights_count(), programId);
    util::set_int("spot_lights_count", Light::spot_lights_count(), programId);
    util::set_int("direction_lights_count", Light::direction_lights_count(), programId);
    for(int i = 0; i < util::Globals::cascade_levels.size(); ++i)
        util::set_float(("cascade_levels[" + std::to_string(i) + "]").c_str(), util::Globals::cascade_levels[i], programId);
    util::set_int("cascade_count", int(util::Globals::cascade_levels.size()) + 1, programId);
}

Shader::PBR::PBR(): ObjectShader("./shader/physicallyBased")
{

}

void Shader::PBR::set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const
{
    util::set_mat("model", model, programId);
    util::set_mat("normal_mat", glm::transpose(glm::inverse(glm::mat3(model))), programId);
    std::vector<Light*> all_lights = Light::get_lights();
    for(Light*& light : all_lights)
    {
        switch(light->type())
        {
        case Light_type::POINT:
        {
            light->set_uniforms("point_lights", programId);
            break;
        }
        case Light_type::SPOT:
        {
            light->set_uniforms("spot_lights", programId);
            break;
        }
        case Light_type::SUN:
        {
            light->set_uniforms("direction_lights", programId);
            break;
        }
        defalut:
            break;
        }
    }
    util::set_int("point_lights_count", Light::point_lights_count(), programId);
    util::set_int("spot_lights_count", Light::spot_lights_count(), programId);
    util::set_int("direction_lights_count", Light::direction_lights_count(), programId);
    util::set_int("cascade_count", int(util::Globals::cascade_levels.size() + 1), programId);
    for(int i = 0; i < util::Globals::cascade_levels.size(); ++i)
        util::set_float(("cascade_levels[" + std::to_string(i) + "]").c_str(), util::Globals::cascade_levels[i], programId);
    util::set_floats("view_pos", camera.position(), programId);
    util::set_float("bias", util::Globals::shadow_bias, programId);
    material.set_uniforms("material", programId);
    Skybox::set_uniforms("skybox", camera, programId);
    util::set_float("bloom", util::Globals::bloom, programId);
    util::set_float("threshold", util::Globals::threshold, programId);
}

Shader::SingleColor::SingleColor(): Shader("./shader/singleColor") 
{
    // Bind current shader's Matices uniform block to Binding Point 0
    // glUniformBlockBinding(program_id, glGetUniformBlockIndex(program_id, "Matrices"), 0);
}

void Shader::SingleColor::set_uniforms(const glm::mat4& model, const glm::vec3& color) const
{
    util::set_mat("model", model, programId);
}

Shader::DepthShader::DepthShader(std::string dir_path): Shader(dir_path)
{
    
}

Shader::CascadeMap::CascadeMap(): DepthShader("./shader/cascadeMap")
{

}

void Shader::CascadeMap::set_uniforms(const glm::mat4& model, const Light& light) const
{
    util::set_mat("model", model, programId);
    util::set_int("dir_light_id", light.index(), programId);
}

Shader::DepthCubemap::DepthCubemap(): DepthShader("./shader/depthCubemap") 
{

}

Shader::Normal::Normal(): Shader("./shader/normal")
{
    // glUniformBlockBinding(program_id, glGetUniformBlockIndex(program_id, "Matrices"), 0);
}

void Shader::Normal::set_uniforms(const Camera& camera, const glm::mat4& model) const
{
    util::set_mat("model", model, programId);
    util::set_mat("normal_mat", glm::transpose(glm::inverse(glm::mat3(model))), programId);
    util::set_float("magnitude", util::Globals::normal_magnitude, programId);
}

Shader::SkyCube::SkyCube(): Shader("./shader/skyCube")
{

}

void Shader::SkyCube::set_uniforms(const Camera& camera, float intensity, GLuint texture_unit) const
{
    util::set_mat("view", glm::transpose(util::Globals::camera.camera_mat()), programId);
    util::set_mat("projection", camera.projection(), programId);
    util::set_int("skybox", texture_unit, programId);
    util::set_float("brightness", intensity, programId);
    util::set_float("threshold", util::Globals::threshold, programId);
}

void Shader::DepthCubemap::set_uniforms(const glm::mat4& model, const Light& light) const
{
    const Point_light& p_light = static_cast<const Point_light&>(light); 
    util::set_mat("model", model, programId);
    std::array<glm::mat4, 6> all_mat = p_light.light_space_mat();
    for(int i = 0; i < 6; ++i)
        util::set_mat(("light_space_mat[" + std::to_string(i) + "]").c_str(), all_mat[i], programId);
    util::set_float("near", light.near(), programId);
    util::set_float("far", light.far(), programId);
    util::set_floats("light_pos", light.position(), programId);
}

Shader::TangentNormal::TangentNormal(): Shader("./shader/tangentNormal")
{

}

void Shader::TangentNormal::set_uniforms(const Camera &camera, const glm::mat4 &model) const
{
    util::set_mat("model", model, programId);
    util::set_mat("normal_mat", glm::inverse(glm::transpose(glm::mat3(model))), programId);
    util::set_float("magnitude", util::Globals::tangent_magnitude, programId);
}

Shader::PostProc* Shader::PostProc::instance = nullptr;

Shader::PostProc::PostProc(): Shader("./shader/postQuad")
{

}

void Shader::PostProc::set_uniforms(GLuint image_unit, GLuint blured_image_unit) const
{
    util::set_int("image", image_unit, programId);
    util::set_int("blured_brightness", blured_image_unit, programId);
    util::set_bool("hdr", util::Globals::hdr, programId);
    util::set_bool("bloom", util::Globals::bloom, programId);
    util::set_float("exposure", util::Globals::exposure, programId);
}

Shader::PostProc *Shader::PostProc::get_instance()
{
    return instance ? instance : (instance = new PostProc());
}

Shader::BloomBlur* Shader::BloomBlur::instance = nullptr;

Shader::BloomBlur::BloomBlur(): Shader("./shader/bloomBlur")
{

}

void Shader::BloomBlur::set_uniforms(GLuint image_unit, bool horizental) const
{
    util::set_int("image", image_unit, programId);
    util::set_bool("horizental", horizental, programId);
}

Shader::BloomBlur *Shader::BloomBlur::get_instance()
{
    return instance ? instance : (instance = new BloomBlur());
}

Shader::SSAOBlur* Shader::SSAOBlur::instance = nullptr;

Shader::SSAOBlur *Shader::SSAOBlur::get_instance()
{
    return instance ? instance : (instance = new SSAOBlur());
}

void Shader::SSAOBlur::set_uniforms() const
{
    util::set_int("ssao_buffer", Model::getTexture("SSAO color buffer").texUnit, programId);
}

Shader::SSAOBlur::SSAOBlur() : Shader("./shader/ssaoBlur")
{
    
}

void Shader::GBuffer::set_uniforms(const Material &material, const Camera &camera, const glm::mat4 &model) const
{
    util::set_mat("model", model, programId);
    util::set_mat("normal_mat", glm::inverse(glm::transpose(glm::mat3(model))), programId);
    material.set_uniforms("material", programId);
    util::set_floats("view_pos", camera.position(), programId);
}

Shader::GBuffer::GBuffer(std::string dir_path): Shader(dir_path)
{
    
}

Shader::GBufferBP* Shader::GBufferBP::instance = nullptr;

Shader::GBufferBP::GBufferBP(): GBuffer("./shader/GBufferBP")
{
    
}

Shader::GBufferBP* Shader::GBufferBP::get_instance()
{
    return instance ? instance : (instance = new GBufferBP()); 
}
Shader::GBufferPBR* Shader::GBufferPBR::instance = nullptr;

Shader::GBufferPBR::GBufferPBR(): GBuffer("./shader/GBufferPBR")
{

}

Shader::GBufferPBR* Shader::GBufferPBR::get_instance()
{
    return instance ? instance : (instance = new GBufferPBR());
}

Shader::TransparentWBPBR* Shader::TransparentWBPBR::instance = nullptr;

Shader::TransparentWBPBR::TransparentWBPBR(): Shader("./shader/transparentWBPBR") { }

Shader::TransparentWBPBR* Shader::TransparentWBPBR::getInstance()
{
    return instance ? instance : (instance = new TransparentWBPBR());
}

void Shader::TransparentWBPBR::setUniforms(const Material& material, const Camera& camera, const glm::mat4& model) const
{
    // 目前这些uniform变量的设置和PBR Shader是一样的
    util::set_mat("model", model, programId);
    util::set_mat("normalMat", glm::transpose(glm::inverse(glm::mat3(model))), programId);
    std::vector<Light*> all_lights = Light::get_lights();
    for(Light*& light : all_lights)
    {
        switch(light->type())
        {
        case Light_type::POINT:
        {
            light->set_uniforms("point_lights", programId);
            break;
        }
        case Light_type::SPOT:
        {
            light->set_uniforms("spot_lights", programId);
            break;
        }
        case Light_type::SUN:
        {
            light->set_uniforms("direction_lights", programId);
            break;
        }
        defalut:
            break;
        }
    }
    util::set_int("point_lights_count", Light::point_lights_count(), programId);
    util::set_int("spot_lights_count", Light::spot_lights_count(), programId);
    util::set_int("direction_lights_count", Light::direction_lights_count(), programId);
    util::set_int("cascade_count", int(util::Globals::cascade_levels.size() + 1), programId);
    for(int i = 0; i < util::Globals::cascade_levels.size(); ++i)
        util::set_float(("cascade_levels[" + std::to_string(i) + "]").c_str(), util::Globals::cascade_levels[i], programId);
    util::set_floats("view_pos", camera.position(), programId);
    util::set_float("bias", util::Globals::shadow_bias, programId);
    material.set_uniforms("material", programId);
    Skybox::set_uniforms("skybox", camera, programId);
    util::set_float("bloom", util::Globals::bloom, programId);
    util::set_float("threshold", util::Globals::threshold, programId);
}

Shader::Composite* Shader::Composite::instance = nullptr;

Shader::Composite::Composite(): Shader("./shader/composite") { }

Shader::Composite* Shader::Composite::getInstance() 
{ 
    return instance ? instance : (instance = new Composite());
}

void Shader::Composite::setUniforms(GLuint accumTexUnit, GLuint revealageTexUnit) const
{
    util::set_int("accum", accumTexUnit, programId);
    util::set_int("revealage", revealageTexUnit, programId);
    util::set_bool("bloom", util::Globals::bloom, programId);
    util::set_float("threshold", util::Globals::threshold, programId);
}

void Shader::SSAO::set_uniforms() const
{
    util::set_int("position_buffer", Model::getTexture("G buffer position buffer").texUnit, programId);
    util::set_int("surface_normal", Model::getTexture("G buffer surface normal buffer").texUnit, programId);
    util::set_int("noise_tex", Model::getTexture("SSAO noisetex").texUnit, programId);
    util::set_float("radius", util::Globals::ssao_radius, programId);
    // Here I take samples when assigning uniforms, which means smaples differs per rendering frame per fragment
    std::array<glm::vec3, 64> samples = util::get_ssao_samples();
    for(int i = 0; i < 64; ++i)
        util::set_floats(("samples[" + std::to_string(i) + "]").c_str(), samples[i], programId);
}

Shader::SSAO *Shader::SSAO::get_instance()
{
    return instance ? instance : (instance = new SSAO());
}

Shader::SSAO* Shader::SSAO::instance = nullptr;

Shader::SSAO::SSAO() : Shader("./shader/ssao")
{

}

void Shader::LightingPass::set_uniforms(const Camera &camera) const
{
    util::set_int("position_buffer", Model::getTexture("G buffer position buffer").texUnit, programId);
    util::set_int("normal_depth", Model::getTexture("G buffer normal_depth buffer").texUnit, programId);
    util::set_int("surface_normal", Model::getTexture("G buffer surface normal buffer").texUnit, programId);
    util::set_int("albedo_specular", Model::getTexture("G buffer albedo_specular buffer").texUnit, programId);
    util::set_int("ambient_metalic", Model::getTexture("G buffer ambient_metalic buffer").texUnit, programId);
    util::set_int("ssao_buffer", Model::getTexture("SSAO color buffer").texUnit, programId);
    util::set_int("point_lights_count", Light::point_lights_count(), programId);
    util::set_int("spot_lights_count", Light::spot_lights_count(), programId);
    util::set_int("direction_lights_count", Light::direction_lights_count(), programId);
    std::vector<Light*> all_lights = Light::get_lights();
    for(Light *& light: all_lights)
    {
        switch(light->type())
        {
        case Light_type::POINT:
        {
            light->set_uniforms("point_lights", programId);
            break;
        }
        case Light_type::SPOT:
        {
            light->set_uniforms("spot_lights", programId);
            break;
        }
        case Light_type::SUN:
        {
            light->set_uniforms("direction_lights", programId);
            break;
        }
        default:
            break;
        }
    }
    Skybox::set_uniforms("skybox", camera, programId);
    util::set_floats("view_pos", camera.position(), programId);
    util::set_float("bias", util::Globals::shadow_bias, programId);
    for(int i = 0; i < util::Globals::cascade_levels.size(); ++i)
        util::set_float(("cascade_levels[" + std::to_string(i) + "]").c_str(), util::Globals::cascade_levels[i], programId);
    util::set_int("cascade_count", static_cast<int>(util::Globals::cascade_levels.size() + 1), programId);
    util::set_float("threshold", util::Globals::threshold, programId);
    util::set_bool("ssao", util::Globals::SSAO, programId);
}

Shader::LightingPass::LightingPass(std::string dir_path): Shader(dir_path)
{
    
}

Shader::LightingPassBP* Shader::LightingPassBP::instance = nullptr;

Shader::LightingPassBP* Shader::LightingPassBP::get_instance()
{
    return instance ? instance : (instance = new LightingPassBP());
}

Shader::LightingPassBP::LightingPassBP(): LightingPass("./shader/lightingPassBP")
{

}

Shader::LightingPassPBR* Shader::LightingPassPBR::instance = nullptr;

Shader::LightingPassPBR* Shader::LightingPassPBR::get_instance()
{
    return instance ? instance : (instance = new LightingPassPBR());
}

Shader::LightingPassPBR::LightingPassPBR(): LightingPass("./shader/lightingPassPBR")
{

}

Shader::HDRI2cubemap* Shader::HDRI2cubemap::instance = nullptr;

Shader::HDRI2cubemap::HDRI2cubemap(): Shader("./shader/HDRI2Cubemap")
{

}

void Shader::HDRI2cubemap::set_uniforms(GLuint HDRI_map_unit, const glm::mat4& view) const
{
    util::set_mat("projection", glm::perspective(glm::radians(90.f), 1.f, 0.1f, 1.f), programId);
    util::set_mat("view", view, programId);
    util::set_int("equirectangularmap", HDRI_map_unit, programId);
}

Shader::HDRI2cubemap* Shader::HDRI2cubemap::get_instance()
{
    return instance ? instance : (instance = new HDRI2cubemap());
}

Shader::Cubemap2Irradiance* Shader::Cubemap2Irradiance::instance = nullptr;

Shader::Cubemap2Irradiance::Cubemap2Irradiance(): Shader("./shader/cubemap2Irradiance")
{

}

Shader::Cubemap2Irradiance* Shader::Cubemap2Irradiance::get_instance()
{
    return instance ? instance : (instance = new Cubemap2Irradiance());
}

void Shader::Cubemap2Irradiance::set_uniforms(const glm::mat4& view, GLuint cubemap_unit) const
{
    util::set_mat("view", view, programId);
    util::set_mat("projection", glm::perspective(glm::radians(90.f), 1.f, 0.1f, 1.f), programId);
    util::set_int("env_map", cubemap_unit, programId);
}

Shader::CubemapPrefilter* Shader::CubemapPrefilter::instance = nullptr;

Shader::CubemapPrefilter::CubemapPrefilter(): Shader("./shader/cubemapPrefilter")
{

}

Shader::CubemapPrefilter* Shader::CubemapPrefilter::get_instance()
{
    return instance ? instance : (instance = new CubemapPrefilter());
}

void Shader::CubemapPrefilter::set_uniforms(const glm::mat4& view, GLuint cubemap_unit, float roughness) const
{
    glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
    util::set_mat("view", view, programId);
    util::set_mat("projection", projection, programId);
    util::set_float("roughness", roughness, programId);
    util::set_int("enviroment_map", cubemap_unit, programId);
}   

Shader::CubemapBRDFIntergral* Shader::CubemapBRDFIntergral::instance = nullptr;

Shader::CubemapBRDFIntergral::CubemapBRDFIntergral(): Shader("./shader/cubemapBRDFIntergral")
{
    
}

Shader::CubemapBRDFIntergral* Shader::CubemapBRDFIntergral::getInstance()
{
    return instance ? instance : (instance = new CubemapBRDFIntergral());
}


void Shader::CubemapBRDFIntergral::set_uniforms() const
{
    // No uniforms to set
}

Shader::FBODebuger* Shader::FBODebuger::instance = nullptr;

Shader::FBODebuger::FBODebuger(): Shader("./shader/FBODebuger")
{

}

Shader::FBODebuger* Shader::FBODebuger::getInstance()
{
    return instance ? instance : (instance = new FBODebuger());
}

void Shader::FBODebuger::set_uniforms(GLuint tex_unit, const glm::mat4& model, bool alpha) const
{
    util::set_mat("model", model, programId);
    util::set_int("fbo_attachment", tex_unit, programId);
    util::set_bool("alpha", alpha, programId);
}

Shader::TextShader* Shader::TextShader::instance = nullptr;

Shader::TextShader::TextShader(): Shader("./shader/textShader")
{

}

Shader::TextShader* Shader::TextShader::get_instance()
{
    return instance ? instance : (instance = new TextShader());
}

void Shader::TextShader::set_uniforms(const glm::mat4& projection, const glm::vec3& color, GLuint bitmap_unit) const
{
    util::set_mat("projection", projection, programId);
    util::set_int("bitmap", bitmap_unit, programId);
    util::set_floats("color", color, programId);
}