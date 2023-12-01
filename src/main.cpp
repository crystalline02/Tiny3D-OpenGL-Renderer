/*Authorized by ZhenKou at 2023.7.24*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backend/imgui_impl_glfw.h>
#include <backend/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "shader.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "model.h"
#include "quad.h"
#include "skybox.h"
#include "globals.h"

int main(int argc, char** argv)
{
	INIT_GLFW();

 	// Init window
	GLFWwindow* window = glfwCreateWindow(util::Globals::camera.width(), 
		util::Globals::camera.height(), 
		"OpenGL renderer", 
		nullptr, 
		nullptr);
	if(window == nullptr)
	{
		std::cout << "Failed to create glfw window!" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	
	// Init glad, loading opengl function
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to load glad!" << std::endl;
		return -1;
	}
	
	// Viewport transform
	glViewport(0, 0, util::Globals::camera.width(), util::Globals::camera.height());

	// Set callback function
	glfwSetFramebufferSizeCallback(window, util::framebuffer_size_callback);
	glfwSetCursorPosCallback(window, util::mouse_callback);
	glfwSetScrollCallback(window, util::scroll_callback);
	glfwSetKeyCallback(window, util::key_callback);

	// Init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	ImGui::StyleColorsDark();
	io.FontGlobalScale = ImGui::GetFrameHeight() * .3f;
	ImGui::GetStyle().ScaleAllSizes(ImGui::GetFrameHeight() * .3f);

	// Model
	Model model("./model/cubes/cubes.obj");

	// Shaders
	Blinn_phong object_shader;
	Single_color color_shader;
	Sky_cube skybox_shader;
	Normal normal_shader;
	Cascade_map cascade_shader;
	Depth_cubemap depth_cube_shader;
	Tangent_normal tangent_shader;
	PBR pbr_shader;

	// Lights
	Point_light light1(glm::vec3(2.f, 1.5f, -2.f), glm::vec3(1.f, .1f, .1f), 4.2f);
	Point_light light4(glm::vec3(-2.f, 1.5f, 2.f), glm::vec3(.1f, .1f, 1.f), 4.2f);
	Direction_light light5(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(0.f, 1.8f, 0.f), glm::vec3(.9373f, .702f, .4745f), .85f);
	Spot_light light6(glm::vec3(-2.f, -2.6f, -1.5f), glm::vec3(2.25f, 1.91f, 2.27f), glm::vec3(.5f, .5f, .5f), 5.f);

	// Scene framebuffer for postprocessing
	util::gen_FBOs();

	while(!glfwWindowShouldClose(window))
	{
		// Process input
		util::process_input(window);

		// Start of imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Render preperation, mainly clear framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);  // 设置ClearColor的值
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);  // 使用ClearColor的值来clearGL_COLOR_BUFFER_BIT,GL_DEPTH_BUFFER_BIT默认为1.f，GL_STENCIL_BUFFER_BIT默认为0
		glBindFramebuffer(GL_FRAMEBUFFER, util::Globals::scene_fbo_ms);
		glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if(util::Globals::deferred_rendering)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, util::Globals::Gbuffer_fbo);
			glClearColor(util::Globals::bg_color.x, util::Globals::bg_color.y, util::Globals::bg_color.z, 0.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		glEnable(GL_MULTISAMPLE);
		Shader::update_uniform_blocks(util::Globals::camera);
		glm::mat4 model_mat(1.f);
		model_mat = glm::rotate(model_mat, glm::radians((float)glfwGetTime() * 15.f), glm::vec3(0.f, 1.f, 0.f));
		// model.set_model(model_mat);
		model.set_cullface(util::Globals::cull_face);
		model.set_blend(util::Globals::blend);

		// Draw depth maps
		model.draw_depthmaps(cascade_shader, depth_cube_shader);
		
		// Draw model.If deferred rendering is enabled, we draw on g-buffers,if not we draw objects directly.
		if(util::Globals::deferred_rendering)
		{
			model.gbuffer_pass(*G_buffer::get_instance(), util::Globals::camera, util::Globals::Gbuffer_fbo);
			Postproc_quad::get_instance()->lighting_pass(*Lighting_pass::get_instance(), util::Globals::camera, util::Globals::scene_fbo_ms);
			if(util::Globals::SSAO)
			{
				Postproc_quad::get_instance()->ssao_pass(*SSAO::get_instance(), *SSAO_blur::get_instance(), util::Globals::ssao_fbo, util::Globals::ssao_blur_fbo);
			}
			model.forward_tranparency(object_shader, util::Globals::camera, util::Globals::scene_fbo_ms);
		}
		else
			model.draw(object_shader, util::Globals::camera, util::Globals::scene_fbo_ms);

		// Draw skybox
		if(util::Globals::skybox) 
			Skybox::get_instance()->draw(skybox_shader, util::Globals::camera, util::Globals::scene_fbo_ms);

		// Post process
		Postproc_quad::get_instance()->draw(*Post_proc::get_instance());

		// Draw outline
		model.draw_outline(color_shader, util::Globals::camera);

		// Draw light
		Light_vertices::get_instance()->draw(color_shader, util::Globals::camera);

		// Draw model normals
		if(util::Globals::visualize_normal)
			model.draw_normals(normal_shader, util::Globals::camera);

		// Draw model tangent normal
		if(util::Globals::visualize_tangent) 
			model.draw_tangent(tangent_shader, util::Globals::camera);

		// Imgui user interface design
		util::imgui_design(model);

		// End of imgui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		// Check and call events and swap buffers
		glfwPollEvents();  // 检测有无按下按键，鼠标操作等等，并执行相应的回调函数
		glfwSwapBuffers(window);  // 双缓冲，把back buffer换到front buffer去，得以显示渲染图
		
		double current_time = glfwGetTime();
		util::Globals::delta_time = current_time - util::Globals::last_time;
		util::Globals::last_time = current_time;
	}
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}