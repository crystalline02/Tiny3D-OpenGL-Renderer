#include "globals.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "camera.h"
#include "shader.h"
#include "light.h"
#include "model.h"
#include "skybox.h"
#include "postproc.h"

#include <chrono>
#include <imgui.h>

bool util::Globals::first_mouse = true,
    util::Globals::blend = false,
    util::Globals::cull_face = false,
    util::Globals::skybox = false,
    util::Globals::visualize_normal = false,
    util::Globals::visualize_tangent = false,
    util::Globals::wireframe = false,
    util::Globals::hdr = false,
    util::Globals::post_process = false,
    util::Globals::bloom = false,
    util::Globals::deferred_rendering = false;
double util::Globals::last_xpos = 0.f, 
    util::Globals::last_ypos = 0.f,
    util::Globals::delta_time = 0.f,
    util::Globals::last_time = 0.f;
float util::Globals::normal_magnitude = 0.2f,
    util::Globals::tangent_magnitude = 0.2f,
    util::Globals::shadow_bias = 0.0005f,
    util::Globals::exposure = 1.f,
    util::Globals::threshold = 1.f;
int util::Globals::cascade_size = 1024,
    util::Globals::cubemap_size = 1024;
GLuint util::Globals::scene_fbo_ms, 
    util::Globals::scene_unit[2],
    util::Globals::pingpong_fbos[2], 
    util::Globals::pingpong_colorbuffers_units[2],
    util::Globals::blur_brightimage_unit,
    util::Globals::Gbuffer_fbo,
    util::Globals::G_color_units[4];

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

glm::mat4 util::face_camera_model(const glm::mat4& camera_mat, glm::vec3 light_pos)
{
    glm::mat4 model_light(camera_mat);
    // 在glm中，对一个矩阵取[]表示取列
    model_light[3][0] = light_pos.x;
    model_light[3][1] = light_pos.y;
    model_light[3][2] = light_pos.z;
    return model_light;
}

void util::create_texture(unsigned int texture_unit, const char* path, bool is_SRGB)
{
    std::cout << "Loading texture form " << path << "......\t";
    auto start = std::chrono::high_resolution_clock::now();
    int width, height, channels;
    GLuint texture_id;
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
    Model::add_texture(path, texture_unit);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}

void util::create_cubemap(GLuint texture_unit, const std::vector<std::string>& faces)
{
    assert(faces.size() == 6);
    std::cout << "Loading sky box from:" << faces[0].substr(0, faces[0].find_last_of("/")) << "......";
    auto start = std::chrono::high_resolution_clock::now();
    GLuint cube_map;
    glGenTextures(1, &cube_map);
    // 应当保证glActiveTexture和glBindTexture一起用，要不然纹理就会乱套了
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
    int width, height, channels;
    for(GLuint i = 0; i < faces.size(); ++i)
    {
        std::string s = faces[i];
        unsigned char* data = stbi_load(s.c_str(), &width, &height, &channels, 0);
        if(data) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else
        {
            std::cout << "Loading failded, error: " << stbi_failure_reason() << std::endl;
            exit(1);
        }
        Model::add_texture(s.c_str(), texture_unit);
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE0);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
}

void util::create_scene_framebuffer_ms(GLuint& fbo, GLuint* scene_units)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint color_attachments[2];
    glGenTextures(2, color_attachments);
    for(unsigned int i = 0; i < 2; ++i)
    {
        scene_units[i] = Model::num_textures();
        Model::add_texture(("scene fbo color attachments" + std::to_string(i)).c_str(), scene_units[i]);
        glActiveTexture(GL_TEXTURE0 + scene_units[i]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color_attachments[i]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, 
            util::Globals::camera.width(), util::Globals::camera.height(), GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

void util::imgui_design()
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
			    ImGui::SliderFloat("intensity", &light->intensity_ptr(), 0.f, 250.f);
                light->set_positon(position);
                break;
            }
            case Light_type::SUN:
            {
                ImGui::SliderFloat("intensity", &light->intensity_ptr(), 0.f, 50.f);
                break;
            }
            case Light_type::SPOT:
            {
                Spot_light* spot_light = static_cast<Spot_light*>(light);
                ImGui::SliderFloat("intensity", &light->intensity_ptr(), 0.f, 250.f);
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
        
    }
    if(ImGui::CollapsingHeader("postprocess"))
    {
        ImGui::Checkbox("posteffects", &Globals::post_process);
        if(Globals::post_process)
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
    ImGui::Checkbox("wireframe", &Globals::wireframe);
	ImGui::Checkbox("blend", &Globals::blend);
	ImGui::Checkbox("cull face", &Globals::cull_face);
	ImGui::End();
}

void util::genFBOs()
{
    create_scene_framebuffer_ms(Globals::scene_fbo_ms, Globals::scene_unit);
    create_pingpong_framebuffer_ms(Globals::pingpong_fbos, Globals::pingpong_colorbuffers_units);
    create_G_frambuffer(Globals::Gbuffer_fbo, Globals::G_color_units);
}

void util::create_G_frambuffer(GLuint &G_fbo, GLuint *G_color_units)
{
    glGenFramebuffers(1, &G_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, G_fbo);

    GLuint g_position, g_normal_depth, g_surface_normal, g_albedo_specular, g_ambient; 
    glGenTextures(1, &g_position);
    G_color_units[0] = Model::num_textures();
    Model::add_texture("G buffer position buffer", G_color_units[0]);
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
    G_color_units[1] = Model::num_textures();
    Model::add_texture("G buffer normal buffer", G_color_units[1]);
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
    G_color_units[2] = Model::num_textures();
    Model::add_texture("G buffer surface normal buffer", G_color_units[2]);
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
    G_color_units[3] = Model::num_textures();
    Model::add_texture("G buffer albedo_specular buffer", G_color_units[3]);
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
    G_color_units[4] = Model::num_textures();
    Model::add_texture("G buffer ambient buffer", G_color_units[4]);
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
        pingpong_texture_units[i] = Model::num_textures();
        Model::add_texture(("pingpong color attchment" + std::to_string(i)).c_str(), pingpong_texture_units[i]);
        glActiveTexture(GL_TEXTURE0 + pingpong_texture_units[i]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, pingpong_color_attchments[i]);
        // pingpong fbo的color buffer需要16bit的，这是因为我们主要用pingpong fbo来做blur，初始给pingpong fbo
        // 的birghtness image就是16bit的（scene fbo），最终blur的结果也应该是16bit的
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, 
            util::Globals::camera.width(), util::Globals::camera.height(), GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

void util::create_cascademap_framebuffer(GLuint &depth_fbo, GLuint &depth_map, GLuint texture_unit)
{
    glGenFramebuffers(1, &depth_fbo);
    glGenTextures(1, &depth_map);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depth_map);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
        Globals::cascade_size, Globals::cascade_size, Globals::cascade_levels.size() + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[4] = {1.f, 1.f, 1.f, 1.f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_map, 0);
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

void util::create_depthcubemap_framebuffer(GLuint& depth_fbo, GLuint& depth_cubemap, GLuint texture_unit)
{
    glGenFramebuffers(1, &depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glGenTextures(1, &depth_cubemap);
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
    for(int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
            util::Globals::cascade_size, util::Globals::cascade_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Depth framebuffer incomplete!\n";
        exit(1);
    }
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