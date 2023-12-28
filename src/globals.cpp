#include "globals.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "camera.h"
#include "shader.h"
#include "light.h"
#include "model.h"
#include "skybox.h"
#include "quad.h"
#include "material.h"

#include <chrono>
#include <random>

#include <imgui.h>


/*
对于不同的编译单元（即.cpp文件），静态成员的初始化顺序可能会有所不同，这取决于链接器如何排列这些单元。因此，
最好避免在不同编译单元中的静态初始化代码之间产生依赖关系。
*/
bool util::Globals::first_mouse = true,
    util::Globals::blend = false,
    util::Globals::cull_face = false,
    util::Globals::skybox = false,
    util::Globals::visualize_normal = false,
    util::Globals::visualize_tangent = false,
    util::Globals::wireframe = false,
    util::Globals::hdr = false,
    util::Globals::bloom = false,
    util::Globals::deferred_rendering = false,
    util::Globals::SSAO = false,
    util::Globals::pbr_mat = true;
double util::Globals::last_xpos = 0.f,
    util::Globals::last_ypos = 0.f,
    util::Globals::delta_time = 0.f,
    util::Globals::last_time = 0.f;
float util::Globals::normal_magnitude = 0.2f,
    util::Globals::tangent_magnitude = 0.2f,
    util::Globals::shadow_bias = 0.0005f,
    util::Globals::exposure = 1.f,
    util::Globals::threshold = 1.f,
    util::Globals::ssao_radius = 0.15f;
int util::Globals::cascade_size = 1024,
    util::Globals::cubemap_size = 1024;
GLuint util::Globals::scene_fbo_ms,
    util::Globals::scene_unit[2],
    util::Globals::pingpong_fbos[2],
    util::Globals::pingpong_colorbuffers_units[2],
    util::Globals::blur_brightimage_unit,
    util::Globals::Gbuffer_fbo,
    util::Globals::G_color_units[5],
    util::Globals::ssao_fbo,
    util::Globals::ssao_color_unit,
    util::Globals::ssao_noisetex_unit,
    util::Globals::ssao_blur_fbo,
    util::Globals::ssao_blur_unit;

// Camera util::Globals::camera(1920, 1080, glm::vec3(0.f, 2.f, 6.f), -90.f, 0.f, 45.f, 0.1f, 100.f, glm::vec3(0.f, 1.f, 0.f));
Camera util::Globals::camera = Camera(1920, 1080, glm::vec3(0.f), 4.f);
glm::vec3 util::Globals::bg_color(0.25f);
std::vector<float> util::Globals::cascade_levels = {util::Globals::camera.far() / 50.f,
    util::Globals::camera.far() / 25.f,
    util::Globals::camera.far() / 10.f,
    util::Globals::camera.far() / 2.f};

void util::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(util::Globals::first_mouse)
	{
		Globals::last_xpos = xpos;
		Globals::last_ypos = ypos;
		Globals::first_mouse = false;
		return;
	}
	double delta_x = xpos - Globals::last_xpos, delta_y = Globals::last_ypos - ypos;
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE))
    {
        if(xpos < 0.1f) glfwSetCursorPos(window, xpos = Globals::camera.width() - .1f, ypos);
        if(xpos > Globals::camera.width()) glfwSetCursorPos(window, xpos = 0.f, ypos);
        if(ypos < 0.1f) glfwSetCursorPos(window, xpos, ypos = Globals::camera.height() - .1f);
        if(ypos > Globals::camera.height()) glfwSetCursorPos(window, xpos, ypos = 0.f);
    }
	Globals::camera.reaction_middle(delta_x, delta_y, !glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE));
    Globals::camera.reaction_shift_middle(-delta_x, -delta_y, glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE));
	Globals::last_xpos = xpos;
	Globals::last_ypos = ypos;
}

void util::scroll_callback(GLFWwindow* window, double delta_x, double delta_y)
{
    Globals::camera.reaction_scroll(-delta_y);
}

void util::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    util::Globals::camera.set_width(width);
    util::Globals::camera.set_height(height);
}

void util::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_RELEASE)
    {
        if(Globals::camera.mode() == ViewMode::FLY)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            Globals::camera.switch_mode(ViewMode::SPIN);
        }
        else if(Globals::camera.mode() == ViewMode::SPIN)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            Globals::camera.switch_mode(ViewMode::FLY);
        }
    }
}

void util::process_input(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Globals::camera.reaction_W(Globals::delta_time);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Globals::camera.reaction_S(Globals::delta_time);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Globals::camera.reaction_A(Globals::delta_time);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Globals::camera.reaction_D(Globals::delta_time);
	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		Globals::camera.reaction_Q(Globals::delta_time);
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		Globals::camera.reaction_E(Globals::delta_time);
}

void APIENTRY util::debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const char* message, const void* userParam)
{
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
    std::cout << message << "(" << id << ")\n";
    switch(source)
    {
        case GL_DEBUG_SOURCE_API: std::cout << "Source: API\n"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "Source: WINDOW_SYSTEM\n"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: HADER_COMPILER\n"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Source: THIRD PARTY\n"; break;
        case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Source: APPLICATION\n"; break;
        case GL_DEBUG_SOURCE_OTHER: std::cout << "Source: OTHER\n"; break;
    }

    switch(type)
    {
        case GL_DEBUG_TYPE_ERROR: std::cout << "Type: ERROR\n"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: DEPRECATED_BEHAVIOR\n"; break;
        case GL_DEBUG_TYPE_MARKER: std::cout << "Type: MARKER\n"; break;
        case GL_DEBUG_TYPE_OTHER: std::cout << "Type: OTHER\n"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Type: PERFORMANCE\n"; break;
        case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Type: POP_GROUP\n"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Type: PUSH_GROUP\n"; break;
        case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Type: PORTABILITY\n"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "Type: UNDEFINED_BEHAVIOR\n"; break;
    }

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH: std::cout << "Severity: HIGH\n"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Severity: MEDIUM\n"; break;
        case GL_DEBUG_SEVERITY_LOW: std::cout << "Severity: LOW\n"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: NOTIFICATION\n"; break;
    }
}

GLenum util::checkGLError_(const char* file, int line)
{
    GLenum error_code;
    while((error_code = glGetError()) != GL_NO_ERROR)
    {
        std::string error_str;
        switch(error_code)
        {
            case GL_INVALID_ENUM: error_str = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: error_str = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: error_str = "GL_INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW: error_str = "GL_INVALID_OPERATION"; break;
            case GL_STACK_UNDERFLOW: error_str = "GL_STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY: error_str = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error_str = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error_str << " | " << file << " (" << line << ")\n";
    }
    return error_code;
}

glm::mat4 util::face_camera_model(const glm::mat4& camera_mat, glm::vec3 light_pos)
{
    glm::mat4 model_light(camera_mat);
    // 在glm中，对一个矩阵取[]表示取列
    model_light[3][0] = light_pos.x;
    model_light[3][1] = light_pos.y;
    model_light[3][2] = light_pos.z;
    return model_light;
}

void util::create_HDRI(const char* path, Texture& texture)
{
    Texture tex_fatch = Model::get_texture(path);
    if(tex_fatch.texname != "Null texture")
    {
        texture = tex_fatch;
        return;
    }

    std::cout << "Loading HDRI from" << path << "......";
    auto start = std::chrono::high_resolution_clock::now();
    stbi_set_flip_vertically_on_load(true);
    int width, height, comp;
    float* data = stbi_loadf(path, &width, &height, &comp, 0);
    if(data)
    {
        texture.texname = path;
        texture.texunit = Model::fatch_new_texunit();
        glGenTextures(1, &texture.texbuffer);
        glActiveTexture(GL_TEXTURE0 + texture.texunit);
        glBindTexture(GL_TEXTURE_2D, texture.texbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);
        Model::add_texture(path, texture);
        // For HDRI, it's OK not to generate mipmap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glActiveTexture(GL_TEXTURE0);
        stbi_image_free(data);
        stbi_set_flip_vertically_on_load(false);
    }
    else
    {
        std::cout << "Error loading " << path << std::endl;
        exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "\tTime: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}

void util::create_texture(const char* path, bool is_SRGB, Texture& texture)
{
    Texture tex_fatch = Model::get_texture(path);
    if(tex_fatch.texname != "Null texture")
    {
        texture = tex_fatch;
        return;
    }

    std::cout << "Loading texture form " << path << "......\t";
    auto start = std::chrono::high_resolution_clock::now();
    int width, height, channels;
    GLuint texture_id, texture_unit;
    texture_unit = Model::fatch_new_texunit();
    glGenTextures(1, &texture_id);
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    // 字符类型表示文本应用char，想用字符类型表示数字应该用signed char[-127, 127]或者unsigned char[0, 255]
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if(data)
    {
        GLenum format, interal_format;
        if(channels == 3)
        {
            // SRGB意味着纹理在被使用时会自动被进行一次gamma re-correct，把SRGB空间的图像转换到linear space
            interal_format = is_SRGB ? GL_SRGB : GL_RGB;
            format = GL_RGB;
        }
        else if(channels == 4)
        {
            interal_format =  is_SRGB ? GL_SRGB_ALPHA : GL_RGBA;
            format = GL_RGBA;
        }
        else
            interal_format = format = GL_RED;
        glTexImage2D(GL_TEXTURE_2D, 0, interal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Loading failed, error:" << stbi_failure_reason() << std::endl;
        exit(1);
    }
    stbi_image_free(data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glActiveTexture(GL_TEXTURE0);
    texture.texname = path;
    texture.texunit = texture_unit;
    texture.texbuffer = texture_id;
    Model::add_texture(path, texture);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}

void util::create_cubemap(const std::vector<std::string>& faces_path, Texture& cubemap_texture,
    Texture& diffuse_irrad_texture, Texture& prefilter_envmap_texture, Texture& BRDF_LUT)
{
    assert(faces_path.size() == 6);
    std::string texname = faces_path[0].substr(0, faces_path[0].find_last_of("\\/"));
    Texture tex_fatch = Model::get_texture(texname.c_str());
    if(tex_fatch.texname != "Null texture")
    {
        cubemap_texture = tex_fatch;
        return;
    }

    std::cout << "Loading sky box from:" << faces_path[0].substr(0, faces_path[0].find_last_of("/")) << "......";
    auto start = std::chrono::high_resolution_clock::now();
    GLuint cubemap_buffer, cubemap_unit;
    cubemap_unit = Model::fatch_new_texunit();
    glGenTextures(1, &cubemap_buffer);
    // 应当保证glActiveTexture和glBindTexture一起用，要不然纹理就会乱套了
    glActiveTexture(GL_TEXTURE0 + cubemap_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_buffer);
    int width, height, channels;
    for(GLuint i = 0; i < faces_path.size(); ++i)
    {
        std::string s = faces_path[i];
        unsigned char* data = stbi_load(s.c_str(), &width, &height, &channels, 0);
        if(data) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else
        {
            std::cout << "Loading failded, error: " << stbi_failure_reason() << std::endl;
            exit(1);
        }
        stbi_image_free(data);
    }
    cubemap_texture.texname = texname;
    cubemap_texture.texunit = cubemap_unit;
    cubemap_texture.texbuffer = cubemap_buffer;
    Model::add_texture(texname.c_str(), cubemap_texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE0);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    // create diffuse irradiance map for this cubemap
    util::create_diffuse_irrad(cubemap_texture, diffuse_irrad_texture);

    // create prefilter environment map for this cubemap
    util::create_prefilter_envmap(cubemap_texture, prefilter_envmap_texture);

    // create brdf LUT for this hdricubemap
    util::create_BRDF_intergral(cubemap_texture, BRDF_LUT);
}

void util::create_cubemap(const char* hdri_path, Texture& hdri_cubemap_texture, Texture& diffuse_irrad_texture,
    Texture& prefilter_envmap_texture, Texture& BRDF_LUT)
{
    std::string ext = std::string(hdri_path).substr(std::string(hdri_path).find_last_of('.'), strlen(hdri_path));
    assert(ext == ".hdr");

    Texture tex_fatch = Model::get_texture(hdri_path);
    if(tex_fatch.texname != "Null texture")
    {
        hdri_cubemap_texture = tex_fatch;
        return;
    }

    Texture hdri_texture;
    util::create_HDRI(hdri_path, hdri_texture);

    // create cubemap
    hdri_cubemap_texture.texname = std::string(hdri_path).substr(0, std::string(hdri_path).find_last_of(".")) +
        "_cubemap" +
        ext;
    glGenTextures(1, &hdri_cubemap_texture.texbuffer);
    hdri_cubemap_texture.texunit = Model::fatch_new_texunit();
    Model::add_texture(hdri_cubemap_texture.texname.c_str(), hdri_cubemap_texture);
    glActiveTexture(GL_TEXTURE0 + hdri_cubemap_texture.texunit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, hdri_cubemap_texture.texbuffer);
    for(int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, 2048, 2048,
            0, GL_RGB, GL_FLOAT, nullptr);  // This cubmap stores color vaules sampled from hdir map, which means it's value extends the range of [0, 1]
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // convert equirectangularmap(HDRI texture) to this cubemap
    glm::mat4 views[6] = {
        glm::lookAt(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))
    };
    GLuint hdri2cubemap_fbo, hdri2cubemap_rbo;
    glGenFramebuffers(1, &hdri2cubemap_fbo);
    glGenRenderbuffers(1, &hdri2cubemap_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, hdri2cubemap_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, hdri2cubemap_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2048, 2048);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdri2cubemap_rbo);
    for(int i = 0; i < 6; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, hdri2cubemap_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            hdri_cubemap_texture.texbuffer, 0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "hdri2cubemap framebuffer incomplete!\n";
            exit(1);
        }
        Skybox::get_instance()->draw_equirectangular_on_cubmap(*Shader::HDRI2cubemap::get_instance(), views[i],
            hdri_texture.texunit, hdri2cubemap_fbo);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    
    // unbind
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    Model::rm_texture(hdri_texture);  // hdir texture is no longer needed, remove it from current program

    // create diffuse irradiance map for this hdircubemap
    util::create_diffuse_irrad(hdri_cubemap_texture, diffuse_irrad_texture);

    // create prefilter environment map for this hdricubemap
    util::create_prefilter_envmap(hdri_cubemap_texture, prefilter_envmap_texture);

    // create brdf LUT for this hdricubemap
    util::create_BRDF_intergral(hdri_cubemap_texture, BRDF_LUT);
}

void util::create_diffuse_irrad(const Texture& cubemap_tex, Texture& diffuse_irrad_tex)
{
    std::cout << "Generating diffuse irradiance map for current skybox cube map......\t";
    auto start = std::chrono::high_resolution_clock::now();
    GLuint cubemap2irradiance_fbo, cubemap2irradiance_rbo;
    glGenFramebuffers(1, &cubemap2irradiance_fbo);
    glGenRenderbuffers(1, &cubemap2irradiance_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, cubemap2irradiance_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, cubemap2irradiance_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 128, 128);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, cubemap2irradiance_rbo);

    {
        unsigned int loc = cubemap_tex.texname.find_last_of('.');
        diffuse_irrad_tex.texname = cubemap_tex.texname.substr(0, loc) + "_irradiance" +
            cubemap_tex.texname.substr(loc, cubemap_tex.texname.size());
    }

    glGenTextures(1, &diffuse_irrad_tex.texbuffer);
    diffuse_irrad_tex.texunit = Model::fatch_new_texunit();

    Model::add_texture(diffuse_irrad_tex.texname.c_str(), diffuse_irrad_tex);
    glActiveTexture(GL_TEXTURE0 + diffuse_irrad_tex.texunit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, diffuse_irrad_tex.texbuffer);
    for(int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glm::mat4 views[6] = {
        glm::lookAt(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))
    };
    for(int i = 0; i < 6; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, cubemap2irradiance_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            diffuse_irrad_tex.texbuffer, 0);
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        Skybox::get_instance()->draw_irradiancemap(*Shader::Cubemap2irradiance::get_instance(), views[i],
            cubemap_tex.texunit, cubemap2irradiance_fbo);
    }

    // Since this framebuffer(to capture diffuse irradiance) is drawed only once during loading a skybox, we
    // can delete renderbuffer and framebuffer however we like(DO NOT delete texture buffer, because its exactly what we need)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glDeleteFramebuffers(1, &cubemap2irradiance_fbo);
    glDeleteRenderbuffers(1, &cubemap2irradiance_rbo);
    glActiveTexture(GL_TEXTURE0);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
}


void util::create_prefilter_envmap(const Texture& cubemap_tex, Texture& prefiltered_envmap)
{
    std::cout << "Generating prefiltered enviroment map......\t";
    auto start = std::chrono::high_resolution_clock::now();
    GLuint prefilter_fbo, prefilter_rbo;
    glGenFramebuffers(1, &prefilter_fbo);
    glGenRenderbuffers(1, &prefilter_rbo);
    {
        GLuint loc = cubemap_tex.texname.find_last_of('.');
        prefiltered_envmap.texname = cubemap_tex.texname.substr(0, loc) + "_prefilter" +
            cubemap_tex.texname.substr(loc, cubemap_tex.texname.size());
    }
    glGenTextures(1, &prefiltered_envmap.texbuffer);
    prefiltered_envmap.texunit = Model::fatch_new_texunit();
    Model::add_texture(prefiltered_envmap.texname.c_str(), prefiltered_envmap);
    glActiveTexture(GL_TEXTURE0 + prefiltered_envmap.texunit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltered_envmap.texbuffer);
    for(int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glm::mat4 views[6] = {
        glm::lookAt(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
        glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))
    };

    int mipmap_levels = 8;
    for(int mip = 0; mip < mipmap_levels; ++mip)
    {
        float roughness = float(mip) / float(mipmap_levels - 1);
        int mip_width = 512 * glm::pow(0.5, mip);
        int mip_height = 512 * glm::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, prefilter_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height);
        glBindFramebuffer(GL_FRAMEBUFFER, prefilter_fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, prefilter_rbo);
        for(int i = 0; i < 6; ++i)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, prefilter_fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                prefiltered_envmap.texbuffer, mip);
            assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
            Skybox::get_instance()->prefilt_cubemap(*Shader::Cubemap_prefilter::get_instance(),
                views[i],
                roughness,
                cubemap_tex.texunit,
                prefilter_fbo, mip_width, mip_height);
        }
    }
    glActiveTexture(GL_TEXTURE0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteRenderbuffers(1, &prefilter_rbo);
    glDeleteFramebuffers(1, &prefilter_fbo);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}

void util::create_BRDF_intergral(const Texture& cubemap_tex, Texture& BRDF_LUT)
{
    std::cout << "Generating BRDF LUT......\t";
    auto start = std::chrono::high_resolution_clock::now();
    // Create framebuffer
    GLuint brdf_intergral_fbo, brdf_intergral_rbo;
    glGenFramebuffers(1, &brdf_intergral_fbo);
    glGenRenderbuffers(1, &brdf_intergral_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, brdf_intergral_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, brdf_intergral_fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, brdf_intergral_rbo);
    
    // create BRDF_LUT
    Texture texture_temp = Model::get_texture("BRDF LUT");
    if(texture_temp.texname != "Null texture") 
    {
        BRDF_LUT = texture_temp;
        glActiveTexture(GL_TEXTURE0 + BRDF_LUT.texunit);
        glBindTexture(GL_TEXTURE_2D, BRDF_LUT.texbuffer);
    }
    else
    {
        BRDF_LUT.texname = "BRDF LUT";
        glGenTextures(1, &BRDF_LUT.texbuffer);
        BRDF_LUT.texunit = Model::fatch_new_texunit();
        Model::add_texture(BRDF_LUT.texname.c_str(), BRDF_LUT);
        glActiveTexture(GL_TEXTURE0 + BRDF_LUT.texunit);
        glBindTexture(GL_TEXTURE_2D, BRDF_LUT.texbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, BRDF_LUT.texbuffer, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    Skybox::get_instance()->BRDF_LUT_intergral(*Shader::Cubemap_BRDFIntergral::get_instance(), 
        brdf_intergral_fbo, 512, 512);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glDeleteFramebuffers(1, &brdf_intergral_fbo);
    glDeleteRenderbuffers(1, &brdf_intergral_rbo);
    glActiveTexture(GL_TEXTURE0);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}

void util::create_scene_framebuffer_ms(GLuint& fbo, GLuint* scene_units)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint color_attachments[2];
    glGenTextures(2, color_attachments);
    for(unsigned int i = 0; i < 2; ++i)
    {
        scene_units[i] = Model::fatch_new_texunit();
        std::string tex_name = "scene fbo color attachments" + std::to_string(i);
        Model::add_texture(tex_name.c_str(), Texture(tex_name, scene_units[i], color_attachments[i]));
        glActiveTexture(GL_TEXTURE0 + scene_units[i]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color_attachments[i]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F,
            util::Globals::camera.width(), util::Globals::camera.height(), GL_TRUE);
        // It's invalid to set below texture paramerters for GL_TEXTURE_2D_MULTISAMPLE, otherwise GL_INVALID_ENUM error will be raised.
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, color_attachments[i], 0);
    }

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8,
        util::Globals::camera.width(), util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Announcing that we will draw on 2 color attachments
    unsigned int draw_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, draw_buffers);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Model framebuffer incomplete!\n";
        exit(1);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void util::imgui_design(Model& model, const Character_Render& text_renderer)
{
    ImGui::Begin("Settings");
	ImGui::ColorEdit3("Backgroud", glm::value_ptr(Globals::bg_color));
	std::vector<Light*> all_lights = Light::get_lights();
	for(unsigned int i = 0; i < all_lights.size(); ++i)
	{
		Light* light = all_lights[i];
		std::string type;
		switch (light->type())
		{
		case Light_type::POINT:
		{
			type = "point";
			break;
		}
		case Light_type::SUN:
		{
			type = "sun";
			break;
		}
		case Light_type::SPOT:
		{
			type = "spot";
			break;
		}
		default:
			break;
		}
		char block_name[32];
		snprintf(block_name, 32, "%s light%d", type.c_str(), i);
		if(ImGui::CollapsingHeader(block_name))
		{
			ImGui::PushID(i);
			ImGui::ColorEdit3("color", &light->color_ptr());
            switch (light->type())
            {
            case Light_type::POINT:
            {
                glm::vec3 position = light->position();
                ImGui::SliderFloat3("position", &position[0], -3.f, 3.f);
			    ImGui::SliderFloat("intensity", &light->intensity_ptr(), 0.f, 450.f);
                light->set_positon(position);
                break;
            }
            case Light_type::SUN:
            {
                ImGui::SliderFloat("intensity", &light->intensity_ptr(), 0.f, 200.f);
                break;
            }
            case Light_type::SPOT:
            {
                Spot_light* spot_light = static_cast<Spot_light*>(light);
                ImGui::SliderFloat("intensity", &light->intensity_ptr(), 0.f, 450.f);
                float inner = glm::degrees(spot_light->cutoff()), outer = glm::degrees(spot_light->outer_cutoff());
                ImGui::SliderFloat("inner", &inner, .1f, 90.f);
                if(inner >= outer) outer = inner;
                ImGui::SliderFloat("outer", &outer, .1f, 90.f);
                if(outer <= inner) inner = outer;
                spot_light->set_cutoff(inner);
                spot_light->set_outer_cutoff(outer);
                break;
            }
            default:
                break;
            }
			ImGui::PopID();
		}
	}
    if(ImGui::CollapsingHeader("enviroment"))
    {
        ImGui::Checkbox("skybox", &Globals::skybox);
        if(Globals::skybox)
        {
            ImGui::SliderFloat("stength", &Skybox::get_instance()->intensity_ptr(), 0.f, 10.f);

            Skybox* skybox = Skybox::get_instance();
            bool affect_scene = skybox->affect_scene();
            ImGui::Checkbox("affect", &affect_scene);
            skybox->set_affect_scene(affect_scene);

            std::vector<std::string> cubemaps = skybox->directories();
            if(ImGui::BeginCombo("cubemap", cubemaps[skybox->selected()].c_str()))
            {
                for(int i = 0; i < cubemaps.size(); ++i)
                    if(ImGui::Selectable(cubemaps[i].c_str()) && i != skybox->selected())  // 当用户点击此item且此item不是上一次选中的，if成立，走下面的逻辑
                        skybox->set_selected(i);
                ImGui::EndCombo();
            }
        }
    }
    if(ImGui::CollapsingHeader("material"))
    {
        std::vector<std::string> mat_type_string = {"blinn phong", "PBR"};
        if(ImGui::BeginCombo("type", mat_type_string[Globals::pbr_mat].c_str()))
        {
            for(int i = 0; i < mat_type_string.size(); ++i)
            {
                if(ImGui::Selectable(mat_type_string[i].c_str()))
                {
                    Mat_type new_type;
                    if(i == 0)
                    {
                        Globals::pbr_mat = false;
                        new_type = Mat_type::Blinn_Phong;
                    }
                    else if (i == 1)
                    {
                        Globals::pbr_mat = true;
                        new_type = Mat_type::PBR;
                    }
                    model.switch_mat_type(new_type);
                }

            }

            ImGui::EndCombo();
        }
    }
    if(ImGui::CollapsingHeader("postprocess"))
    {
        ImGui::Checkbox("HDR", &Globals::hdr);
        if(Globals::hdr)
        {
            ImGui::SameLine();
            ImGui::SliderFloat("exposure", &Globals::exposure, 0.f, 5.f);
        }
        ImGui::Checkbox("bloom", &Globals::bloom);
        if(Globals::bloom)
        {
            ImGui::SameLine();
            ImGui::SliderFloat("threshold", &Globals::threshold, 0.f, 5.f);
        }
    }
    std::vector<std::string> model_dirs = Model::all_models();
    if(ImGui::BeginCombo("model", model_dirs[Model::seleted_model()].c_str()))
    {
        for(int i = 0; i < model_dirs.size(); ++i)
            if(ImGui::Selectable(model_dirs[i].c_str()))
                Model::set_selected(i);
        ImGui::EndCombo();
    }
    ImGui::Checkbox("normal", &Globals::visualize_normal);
    if(Globals::visualize_normal)
    {
        ImGui::SameLine();
        ImGui::PushID("normal_length");
        ImGui::SliderFloat("length", &Globals::normal_magnitude, 0.01f, .5f, "%.3f");
        ImGui::PopID();
    }
    ImGui::Checkbox("tangent", &Globals::visualize_tangent);
    if(Globals::visualize_tangent)
    {
        ImGui::SameLine();
        ImGui::PushID("tangent_length");
        ImGui::SliderFloat("length", &Globals::tangent_magnitude, 0.01f, 1.f);
        ImGui::PopID();
    }
    ImGui::SliderFloat("shadow bias", &Globals::shadow_bias, 0.0001f, 0.05f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
    if(ImGui::BeginCombo("cascade size", (std::to_string(Globals::cascade_size) + "px").c_str()))
    {
        for(int i = 6; i < 13; ++i)
        {
            if(ImGui::Selectable((std::to_string(int(glm::pow(2, i))) + "px").c_str()))
            {
                Globals::cascade_size = glm::pow(2, i);
                std::vector<Light*> all_lights = Light::get_lights();
                for(Light*& light: all_lights)
                    if(light->type() == Light_type::SUN) light->resize_depthmap(Globals::cascade_size);
            }
        }
        ImGui::EndCombo();
    }
    if(ImGui::BeginCombo("cubemap size", (std::to_string(Globals::cubemap_size) + "px").c_str()))
    {
        for(int i = 6; i < 13; ++i)
        {
            if(ImGui::Selectable((std::to_string(int(glm::pow(2, i))) + "px").c_str()))
            {
                Globals::cubemap_size = glm::pow(2, i);
                std::vector<Light*> all_lights = Light::get_lights();
                for(Light*& light: all_lights)
                    if(light->type() == Light_type::POINT || light->type() == Light_type::SPOT) light->resize_depthmap(Globals::cubemap_size);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Checkbox("deferred rendering(experimental)", &Globals::deferred_rendering);
    if(Globals::deferred_rendering)
    {
        ImGui::Checkbox("SSAO", &Globals::SSAO);
        if(Globals::SSAO)
        {
            ImGui::SameLine();
            ImGui::SliderFloat("radius", &Globals::ssao_radius, 0.01f, 0.3f);
        }
    }
    ImGui::Checkbox("wireframe", &Globals::wireframe);
	ImGui::Checkbox("blend", &Globals::blend);
	ImGui::Checkbox("cull face", &Globals::cull_face);
	ImGui::End();

    Shader::Text_shader* text_shader = Shader::Text_shader::get_instance();
    static int fps = 0;
    double current_time = glfwGetTime();
    if(std::abs(current_time - std::round(current_time)) < 0.01)
        fps = (1.0 / Globals::delta_time);
    text_renderer.render_text(*text_shader, "FPS: " + std::to_string(fps), 
        Globals::camera.width() - 135, 
        Globals::camera.height() - 40, .65f, {0.f, 1.f, 0.05f});
}

std::array<glm::vec3, 64> util::get_ssao_samples()
{
    std::array<glm::vec3, 64> samples;
    std::uniform_real_distribution<float> rand(0.f, 1.f);
    std::default_random_engine generator;
    for(unsigned int i = 0; i < 64; ++i)
    {
        float scale = float(i) / 64.f;
        scale = .1f * (1.f - scale * scale) + scale * scale * 1.f;
        glm::vec3 sample = glm::vec3(rand(generator) * 2.f - 1.f,
            rand(generator) * 2.f - 1.f,
            rand(generator)
        );
        sample = glm::normalize(sample) * scale;
        samples[i] = sample;
    }
    return samples;
}

void util::gen_FBOs()
{
    create_scene_framebuffer_ms(Globals::scene_fbo_ms, Globals::scene_unit);
    create_pingpong_framebuffer_ms(Globals::pingpong_fbos, Globals::pingpong_colorbuffers_units);
    create_G_frambuffer(Globals::Gbuffer_fbo, Globals::G_color_units);
    create_ssao_framebuffer(Globals::ssao_fbo, Globals::ssao_blur_fbo,
        Globals::ssao_color_unit, Globals::ssao_blur_unit, Globals::ssao_noisetex_unit);
}

void util::create_G_frambuffer(GLuint &G_fbo, GLuint *G_color_units)
{
    glGenFramebuffers(1, &G_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, G_fbo);

    GLuint g_position, g_normal_depth, g_surface_normal, g_albedo_specular, g_ambient;
    glGenTextures(1, &g_position);
    G_color_units[0] = Model::fatch_new_texunit();
    Model::add_texture("G buffer position buffer", Texture("G buffer position buffer", G_color_units[0],
        g_position));
    glActiveTexture(GL_TEXTURE0 + G_color_units[0]);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position, 0);

    glGenTextures(1, &g_normal_depth);
    G_color_units[1] = Model::fatch_new_texunit();
    Model::add_texture("G buffer normal buffer", Texture("G buffer normal buffer", G_color_units[1],
        g_normal_depth));
    glActiveTexture(GL_TEXTURE0 + G_color_units[1]);
    glBindTexture(GL_TEXTURE_2D, g_normal_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal_depth, 0);

    glGenTextures(1, &g_surface_normal);
    G_color_units[2] = Model::fatch_new_texunit();
    Model::add_texture("G buffer surface normal buffer", Texture("G buffer surface normal buffer",
        G_color_units[2], g_surface_normal));
    glActiveTexture(GL_TEXTURE0 + G_color_units[2]);
    glBindTexture(GL_TEXTURE_2D, g_surface_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_surface_normal, 0);

    glGenTextures(1, &g_albedo_specular);
    G_color_units[3] = Model::fatch_new_texunit();
    Model::add_texture("G buffer albedo_specular buffer", Texture("G buffer albedo_specular buffer",
         G_color_units[3], g_albedo_specular));
    glActiveTexture(GL_TEXTURE0 + G_color_units[3]);
    glBindTexture(GL_TEXTURE_2D, g_albedo_specular);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, g_albedo_specular, 0);

    glGenTextures(1, &g_ambient);
    G_color_units[4] = Model::fatch_new_texunit();
    Model::add_texture("G buffer ambient buffer", Texture("G buffer ambient buffer", G_color_units[4], g_ambient));
    glActiveTexture(GL_TEXTURE0 + G_color_units[4]);
    glBindTexture(GL_TEXTURE_2D, g_ambient);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, g_ambient, 0);

    unsigned int color_attachments[5] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
    glDrawBuffers(5, color_attachments);

    GLuint g_rbo;
    glGenRenderbuffers(1, &g_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, g_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, util::Globals::camera.width(), util::Globals::camera.height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, g_rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "G framebuffer incomplete!\n";
        exit(1);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void util::create_pingpong_framebuffer_ms(GLuint *pingpong_fbos, GLuint *pingpong_texture_units)
{
    GLuint pingpong_color_attchments[2];
    glGenFramebuffers(2, pingpong_fbos);
    glGenTextures(2, pingpong_color_attchments);
    for(int i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpong_fbos[i]);
        pingpong_texture_units[i] = Model::fatch_new_texunit();
        std::string tex_name = "pingpong color attchment" + std::to_string(i);
        Model::add_texture(tex_name.c_str(), Texture(tex_name, pingpong_texture_units[i],
            pingpong_color_attchments[i]));
        glActiveTexture(GL_TEXTURE0 + pingpong_texture_units[i]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, pingpong_color_attchments[i]);
        // pingpong fbo的color buffer需要16bit的，这是因为我们主要用pingpong fbo来做blur，初始给pingpong fbo
        // 的birghtness image就是16bit的（scene fbo），最终blur的结果也应该是16bit的
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F,
            util::Globals::camera.width(), util::Globals::camera.height(), GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, pingpong_color_attchments[i], 0);
        // No need for depth or stencil attachment as what we only care about is color buffer
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Pingpong frame buffer incomplete!\n";
            exit(1);
        }
    }
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void util::create_cascademap_framebuffer(GLuint &depth_fbo, Texture &texture, const char* tex_name)
{
    Texture tex_fatch = Model::get_texture(tex_name);
    if(tex_fatch.texname != "Null texture")
    {
        texture = tex_fatch;
        return;
    }

    GLuint depth_buffer, depth_unit;
    depth_unit = Model::fatch_new_texunit();
    glGenFramebuffers(1, &depth_fbo);
    glGenTextures(1, &depth_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glActiveTexture(GL_TEXTURE0 + depth_unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depth_buffer);
    texture.texname = tex_name;
    texture.texbuffer = depth_buffer;
    texture.texunit = depth_unit;
    Model::add_texture(tex_name, texture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
        Globals::cascade_size, Globals::cascade_size, Globals::cascade_levels.size() + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[4] = {1.f, 1.f, 1.f, 1.f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_buffer, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Depth framebuffer incomplete!\n";
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
}

void util::create_depthcubemap_framebuffer(GLuint& depth_fbo, Texture& texture, const char* tex_name)
{
    Texture tex_fatch = Model::get_texture(tex_name);
    if(tex_fatch.texname != "Null texture")
    {
        texture = tex_fatch;
        return;
    }

    GLuint depth_cubemap_buffer, depth_cubemap_unit;
    depth_cubemap_unit = Model::fatch_new_texunit();
    glGenFramebuffers(1, &depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glGenTextures(1, &depth_cubemap_buffer);
    glActiveTexture(GL_TEXTURE0 + depth_cubemap_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap_buffer);
    texture.texname = tex_name;
    texture.texbuffer = depth_cubemap_buffer;
    texture.texunit = depth_cubemap_unit;
    Model::add_texture(tex_name, texture);
    for(int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            util::Globals::cascade_size, util::Globals::cascade_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap_buffer, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Depth framebuffer incomplete!\n";
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void util::create_ssao_framebuffer(GLuint &ssao_fbo, GLuint &ssao_blur_fbo, GLuint &ssao_color_unit,
    GLuint &ssao_blur_unit, GLuint &ssao_noisetex_unit)
{
    // create ssao_noisetex used to generate random vector to generate tiled TBN matrix for placing the ssao samples
    std::array<glm::vec3, 16> rand_vector;
    std::uniform_real_distribution<float> rand(0.f, 1.f);
    std::default_random_engine generator;
    for(int i = 0; i < 16; ++i)
    {
        rand_vector[i] = glm::vec3(rand(generator) * 2.f - 1.f,
            rand(generator) * 2.f - 1.f,
            0.f);
    }

    GLuint noise_texid;
    glGenTextures(1, &noise_texid);
    ssao_noisetex_unit = Model::fatch_new_texunit();
    Model::add_texture("SSAO noisetex", Texture("SSAO noisetex", ssao_noisetex_unit, noise_texid));
    glActiveTexture(GL_TEXTURE0 + ssao_noisetex_unit);
    glBindTexture(GL_TEXTURE_2D, noise_texid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &rand_vector[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // create ssao framebuffer
    glGenFramebuffers(1, &ssao_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
    GLuint ssao_color_buffer;
    glGenTextures(1, &ssao_color_buffer);
    ssao_color_unit = Model::fatch_new_texunit();
    Model::add_texture("SSAO color buffer", Texture("SSAO color buffer", ssao_color_unit, ssao_color_buffer));
    glActiveTexture(GL_TEXTURE0 + ssao_color_unit);
    glBindTexture(GL_TEXTURE_2D, ssao_color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, util::Globals::camera.width(), util::Globals::camera.height(),
        0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_color_buffer, 0);

    // create ssao_blur framebuffer
    glGenFramebuffers(1, &ssao_blur_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo);
    GLuint ssao_blur_buffer;
    glGenTextures(1, &ssao_blur_buffer);
    ssao_blur_unit = Model::fatch_new_texunit();
    Model::add_texture("SSAO blur buffer", Texture("SSAO blur buffer", ssao_blur_unit, ssao_blur_buffer));
    glActiveTexture(GL_TEXTURE0 + ssao_blur_unit);
    glBindTexture(GL_TEXTURE_2D, ssao_blur_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, util::Globals::camera.width(), util::Globals::camera.height(), 0,
        GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_buffer, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::array<glm::vec3, 8> util::get_frustum_corner_world(const Camera& camera, float near, float far)
{
    assert(near >= camera.near() && far <= camera.far());
    glm::mat4 perspective = glm::perspective(glm::radians(camera.fov()), camera.aspect_ratio(), near, far);
    glm::mat4 view = camera.lookat();
    std::array<glm::vec3, 8> ret;
    unsigned int i = 0;
    for(float x = -1; x <= 1; x += 2.f)
    {
        for(float y = -1; y <= 1; y += 2.f)
        {
            for(float z = -1; z <= 1; z += 2.f)
            {
                glm::vec4 pos_world = glm::inverse(perspective * view) * glm::vec4(x, y, z, 1.f);
                ret[i++] = glm::vec3(pos_world / pos_world.w);
            }
        }
    }
    return ret;
}