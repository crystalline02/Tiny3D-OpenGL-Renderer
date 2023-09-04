#include "shader.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "material.h"
#include "light.h"
#include "camera.h"
#include "globals.h"
#include "skybox.h"

GLuint Shader::ubo_matrices = -1, Shader::ubo_fn = -1, Shader::ubo_light_matrices = -1;

Shader::Shader(std::string dir_path): m_dir(dir_path)
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
            if(geometry_shader_path == "./shader/cascade_map/geometry_shader.gs")
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

    program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    if(geometry_shader_id != -1) glAttachShader(program_id, geometry_shader_id);
    glLinkProgram(program_id);
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program_id, 512, nullptr, infolog);
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
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_matrices, 0, 2 * sizeof(glm::mat4));  // Bind m_ubo_matrics to Binding Point 0
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_fn);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(float), nullptr, GL_STATIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, ubo_fn, 0, 2 * sizeof(float));  // Bind m_ubo_FN to Binding point 1
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_light_matrices);
        glBufferData(GL_UNIFORM_BUFFER, 80 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, ubo_light_matrices, 0, 80 * sizeof(glm::mat4));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);  // Unbind
    }
}

void Shader::set_material(const char* name, const Material& material) const
{
    material.set_uniforms(name, program_id);
}

void Shader::set_globals() const
{
    util::set_float("near", util::Globals::camera.near(), program_id);
    util::set_float("far", util::Globals::camera.far(), program_id);
    util::set_float("bias", util::Globals::shadow_bias, program_id);
}

void Shader::set_single_color(const glm::vec3& color) const
{
    util::set_floats("single_color", color, program_id);
}

void Shader::update_uniform_blocks(const Camera& camera)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera.lookat()));  // Fill buffer for view matrix
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera.projection()));  // Fill buffer for projection matrix

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_fn);
    float n = camera.near(), f = camera.far();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &f);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float), sizeof(float), &n);
    
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_light_matrices);
    std::vector<Light*> all_lights = Light::get_lights();
    for(Light*& light: all_lights)
    {
        if(light->type() == Light_type::SUN)
        {
            Direction_light* dir_light = static_cast<Direction_light*>(light);
            std::vector<glm::mat4> light_matrices = dir_light->light_space_mat(camera);
            assert(light_matrices.size() <= 10);
            glBufferSubData(GL_UNIFORM_BUFFER, dir_light->index() * 10 * sizeof(glm::mat4), light_matrices.size() * sizeof(glm::mat4), light_matrices.data());
        }
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::set_model(const glm::mat4& model) const
{
    util::set_mat("model", model, program_id);
}

void Shader::set_normal_mat(const glm::mat4& model) const
{
    util::set_mat("normal_mat", glm::transpose(glm::inverse(glm::mat3(model))), program_id);  // 要把normal转换到view space下，注意在glm中非常奇怪，矩阵乘法A * B在底层实现实际上是B * A
}

void Shader::set_viewpos(const glm::vec3& view_pos) const
{
    util::set_floats("view_pos", view_pos, program_id);
}

Blinn_phong::Blinn_phong(): Shader("./shader/blinn_phong") 
{     

}

void Blinn_phong::set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const
{
    util::set_mat("model", model, program_id);
    util::set_mat("normal_mat", glm::transpose(glm::inverse(glm::mat3(model))), program_id);
    util::set_floats("view_pos", camera.position(), program_id);
    util::set_float("bias", util::Globals::shadow_bias, program_id);
    std::vector<Light*> all_lights = Light::get_lights();
    for(Light*& light : all_lights)
    {
        switch(light->type())
        {
        case Light_type::POINT:
        {
            light->set_uniforms("point_lights", program_id);
            break;
        }
        case Light_type::SPOT:
        {
            light->set_uniforms("spot_lights", program_id);
            break;
        }
        case Light_type::SUN:
        {
            light->set_uniforms("direction_lights", program_id);
            break;
        }
        defalut:
            break;
        }
    }
    util::set_bool("bloom", util::Globals::bloom, program_id);
    util::set_bool("gamma_correct", !util::Globals::post_process, program_id);
    util::set_float("threshold", util::Globals::threshold, program_id);
    util::set_int("point_lights_count", Light::point_lights_count(), program_id);
    util::set_int("spot_lights_count", Light::spot_lights_count(), program_id);
    util::set_int("direction_lights_count", Light::direction_lights_count(), program_id);
    for(int i = 0; i < util::Globals::cascade_levels.size(); ++i)
        util::set_float(("cascade_levels[" + std::to_string(i) + "]").c_str(), util::Globals::cascade_levels[i], program_id);
    util::set_int("cascade_count", int(util::Globals::cascade_levels.size()) + 1, program_id);
    material.set_uniforms("material", program_id);
    Skybox::set_uniforms("skybox", camera, program_id);
}

Single_color::Single_color(): Shader("./shader/single_color") 
{
    // Bind current shader's Matices uniform block to Binding Point 0
    // glUniformBlockBinding(program_id, glGetUniformBlockIndex(program_id, "Matrices"), 0);
}

void Single_color::set_uniforms(const glm::mat4& model, const glm::vec3& color) const
{
    util::set_mat("model", model, program_id);
}

Depth_shader::Depth_shader(std::string dir_path): Shader(dir_path)
{

}

Cascade_map::Cascade_map(): Depth_shader("./shader/cascade_map")
{

}

void Cascade_map::set_uniforms(const glm::mat4& model, const Light& light) const
{
    util::set_mat("model", model, program_id);
    util::set_int("dir_light_id", light.index(), program_id);
}

Depth_cubemap::Depth_cubemap(): Depth_shader("./shader/depth_cubemap") 
{

}

Normal::Normal(): Shader("./shader/normal")
{
    // glUniformBlockBinding(program_id, glGetUniformBlockIndex(program_id, "Matrices"), 0);
}

void Normal::set_uniforms(const Camera& camera, const glm::mat4& model) const
{
    util::set_mat("model", model, program_id);
    util::set_mat("normal_mat", glm::transpose(glm::inverse(glm::mat3(model))), program_id);
    util::set_float("magnitude", util::Globals::normal_magnitude, program_id);
}

Sky_cube::Sky_cube(): Shader("./shader/sky_cube")
{

}

void Sky_cube::set_uniforms(const Camera& camera, float intensity, GLuint texture_unit) const
{
    util::set_mat("view", glm::transpose(util::Globals::camera.camera_mat()), program_id);
    util::set_mat("projection", camera.projection(), program_id);
    util::set_int("skybox", texture_unit, program_id);
    util::set_float("brightness", intensity, program_id);
    util::set_float("threshold", util::Globals::threshold, program_id);
}

void Depth_cubemap::set_uniforms(const glm::mat4& model, const Light& light) const
{
    const Point_light& p_light = static_cast<const Point_light&>(light); 
    util::set_mat("model", model, program_id);
    std::array<glm::mat4, 6> all_mat = p_light.light_space_mat();
    for(int i = 0; i < 6; ++i)
        util::set_mat(("light_space_mat[" + std::to_string(i) + "]").c_str(), all_mat[i], program_id);
    util::set_float("near", light.near(), program_id);
    util::set_float("far", light.far(), program_id);
    util::set_floats("light_pos", light.position(), program_id);
}

Tangent_normal::Tangent_normal(): Shader("./shader/tangent_normal")
{

}

void Tangent_normal::set_uniforms(const Camera &camera, const glm::mat4 &model) const
{
    util::set_mat("model", model, program_id);
    util::set_mat("normal_mat", glm::inverse(glm::transpose(glm::mat3(model))), program_id);
    util::set_float("magnitude", util::Globals::tangent_magnitude, program_id);
}

Post_proc* Post_proc::instance = nullptr;

Post_proc::Post_proc(): Shader("./shader/post_quad")
{

}

void Post_proc::set_uniforms(GLuint image_unit, GLuint blured_image_unit) const
{
    util::set_int("image", image_unit, program_id);
    util::set_int("blured_brightness", blured_image_unit, program_id);
    util::set_bool("hdr", util::Globals::hdr, program_id);
    util::set_bool("gamma_correct", util::Globals::post_process, program_id);
    util::set_bool("bloom", util::Globals::bloom, program_id);
    util::set_float("exposure", util::Globals::exposure, program_id);
}

Post_proc *Post_proc::get_instance()
{
    return instance ? instance : new Post_proc();
}

Blur* Blur::instance = nullptr;

Blur::Blur(): Shader("./shader/blur")
{

}

void Blur::set_uniforms(GLuint image_unit, bool horizental) const
{
    util::set_int("image", image_unit, program_id);
    util::set_bool("horizental", horizental, program_id);
}

Blur *Blur::get_instance()
{
    return instance ? instance : new Blur();
}

void G_buffer::set_uniforms(const Material &material, const Camera &camera, const glm::mat4 &model) const
{
    util::set_mat("model", model, program_id);
    util::set_mat("normal_mat", glm::inverse(glm::transpose(glm::mat3(model))), program_id);
    material.set_uniforms("material", program_id);
    util::set_floats("view_pos", camera.position(), program_id);
}

G_buffer* G_buffer::instance = nullptr;

G_buffer::G_buffer() : Shader("./shader/G_buffer")
{
    
}

G_buffer *G_buffer::get_instance()
{
    return instance ? instance : new G_buffer();
}

void Lighting_pass::set_uniforms(const Camera &camera) const
{
    util::set_int("position_buffer", util::Globals::G_color_units[0], program_id);
    util::set_int("normal_depth", util::Globals::G_color_units[1], program_id);
    util::set_int("surface_normal", util::Globals::G_color_units[2], program_id);
    util::set_int("albedo_specular", util::Globals::G_color_units[3], program_id);
    util::set_int("ambient_buffer", util::Globals::G_color_units[4], program_id);
    util::set_int("point_lights_count", Light::point_lights_count(), program_id);
    util::set_int("spot_lights_count", Light::spot_lights_count(), program_id);
    util::set_int("direction_lights_count", Light::direction_lights_count(), program_id);
    std::vector<Light*> all_lights = Light::get_lights();
    for(Light *& light: all_lights)
    {
        switch(light->type())
        {
        case Light_type::POINT:
        {
            light->set_uniforms("point_lights", program_id);
            break;
        }
        case Light_type::SPOT:
        {
            light->set_uniforms("spot_lights", program_id);
            break;
        }
        case Light_type::SUN:
        {
            light->set_uniforms("direction_lights", program_id);
            break;
        }
        default:
            break;
        }
    }
    Skybox::set_uniforms("skybox", camera, program_id);
    util::set_floats("view_pos", camera.position(), program_id);
    util::set_float("bias", util::Globals::shadow_bias, program_id);
    for(int i = 0; i < util::Globals::cascade_levels.size(); ++i)
        util::set_float(("cascade_levels[" + std::to_string(i) + "]").c_str(), util::Globals::cascade_levels[i], program_id);
    util::set_int("cascade_count", util::Globals::cascade_levels.size(), program_id);
    util::set_int("gamma_correct", !util::Globals::post_process, program_id);
    util::set_float("threshold", util::Globals::threshold, program_id);
}

Lighting_pass *Lighting_pass::get_instance()
{
    return instance ? instance : new Lighting_pass();
}

Lighting_pass* Lighting_pass::instance = nullptr;

Lighting_pass::Lighting_pass() : Shader("./shader/lighting_pass")
{
    
}