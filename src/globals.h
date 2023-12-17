#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>


class Light;
class Camera;
class Light_vertices;
class Model;
struct Texture;

#define INIT_GLFW() \
	glfwInit(); \
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); \
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); \
    glfwWindowHint(GLFW_SAMPLES, 4); \
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE)

namespace util
{
    struct Globals
    {
        static bool first_mouse, blend, cull_face, skybox, visualize_normal, wireframe, visualize_tangent,
            hdr, bloom, deferred_rendering, SSAO, pbr_mat;
        static double last_xpos, last_ypos;
        static int cascade_size, cubemap_size;
        static GLuint scene_fbo_ms, scene_unit[2], pingpong_fbos[2], pingpong_colorbuffers_units[2], 
            blur_brightimage_unit, Gbuffer_fbo, G_color_units[5], ssao_fbo, ssao_color_unit, ssao_noisetex_unit,
            ssao_blur_fbo, ssao_blur_unit;
        static Camera camera;
        static double delta_time, last_time;
        static float normal_magnitude, shadow_bias, tangent_magnitude, exposure, threshold, ssao_radius;
        static glm::vec3 bg_color;
        static std::vector<float> cascade_levels;
    };
    void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double delta_x, double delta_y);
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void process_input(GLFWwindow* window);
    glm::mat4 face_camera_model(const glm::mat4& camera_mat, glm::vec3 light_pos);

    inline void print_row(const std::vector<std::string>& texts, int col_width)
    {
        std::cout << "|";
        for(const std::string& s: texts)
        {
            int pad_left = (s.size() + col_width) * 0.5f;
            int pad_right = col_width - pad_left;
            std::cout << std::setw(pad_left) << s << std::setw(pad_right) << " " << "|";
        }
        std::cout << std::endl;
    }
    
    inline GLuint create_vbo_2_vao(float* data, GLuint size)
    {
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return VBO;
    }

    inline void print_mat(glm::mat4 mat)
    {
        for(int i = 0; i < 4; ++i)
        {
            for(int j = 0; j < 4; ++j)
                std::cout << mat[i][j] << " ";
            std::cout << "\n";
        }
    }

    inline void print_mat(glm::mat3 mat)
    {
        for(int i = 0; i < 3; ++i)
        {
            for(int j = 0; j < 3; ++j)
                std::cout << mat[i][j] << " ";
            std::cout << "\n";
        }
    }


    inline void set_float(const char* name, float value, GLuint program_id) 
    { 
        GLint location = glGetUniformLocation(program_id, name);
        if(location != -1) glUniform1f(location, value);
    }
    inline void set_int(const char* name, int value, GLuint program_id) 
    { 
        GLint location = glGetUniformLocation(program_id, name);
        if(location != -1) glUniform1i(location, value); 
    }

    inline void set_bool(const char* name, bool value, GLuint program_id) 
    { 
        GLint location = glGetUniformLocation(program_id, name);
        if(location != -1) glUniform1i(location, value); 
    }

    inline void set_floats(const char* name, const glm::vec3& vec3, GLuint program_id)
    { 
        GLint location = glGetUniformLocation(program_id, name);
        if(location != -1) glUniform3f(location, vec3.x, vec3.y, vec3.z); 
    }

    inline void set_floats(const char* name, const glm::vec4& vec4, GLuint program_id)
    { 
        GLint location = glGetUniformLocation(program_id, name);
        if(location != -1) glUniform4f(location, vec4.x, vec4.y, vec4.z, vec4.w); 
    }

    inline void set_mat(const char* name, const glm::mat4& mat4, GLuint program_id) 
    { 
        GLint location = glGetUniformLocation(program_id, name);
        if(location != -1) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat4)); 
    }

    inline void set_mat(const char* name, const glm::mat3& mat3, GLuint program_id) 
    { 
        GLint location = glGetUniformLocation(program_id, name);
        if(location != -1) glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat3)); 
    }

    void gen_FBOs();
    void create_G_frambuffer(GLuint &G_fbo, GLuint *G_color_units);
    void create_pingpong_framebuffer_ms(GLuint *pingpong_fbos, GLuint* pingpong_texture_units);
    void create_cascademap_framebuffer(GLuint &depth_fbo, Texture &texture, const char* tex_name);
    void create_depthcubemap_framebuffer(GLuint& depth_fbo, Texture& texture, const char* tex_name);
    void create_ssao_framebuffer(GLuint &ssao_fbo, GLuint &ssao_blur_fbo,
        GLuint &ssao_color_unit, GLuint &ssao_blur_unit, GLuint &ssao_noisetex_unit);
    void create_HDRI(const char* path, Texture& texture);
    void create_texture(const char* path, bool is_SRGB, Texture& texture);
    void create_cubemap(const std::vector<std::string>& faces_path, Texture& texture, Texture& diffuse_irrad_texture);
    void create_cubemap(const char* hdri_path, Texture& hdri_cubemap_texture, Texture& diffuse_irrad_texture);
    void create_diffuse_irrad(const Texture& cubemap_tex, Texture& diffuse_irrad_tex);
    void create_scene_framebuffer_ms(GLuint& fbo, GLuint* scene_units);
    void imgui_design(Model &model);
    std::array<glm::vec3, 64> get_ssao_samples();
    std::array<glm::vec3, 8> get_frustum_corner_world(const Camera& camera, float near, float far);
}
