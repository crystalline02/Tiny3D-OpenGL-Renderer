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
#include "fboDebuger.h"
#include "fboManager.h"
#include "TAAManager.h"

int main(int argc, char** argv)
{
	INIT_GLFW();

 	// Init window
	GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, 
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
	glfwMakeContextCurrent(window);  // Create context
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	
	// Init glad, loading opengl function
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to load glad!" << std::endl;
		return -1;
	}

	// Code to check whether debug context is enabled.If debug context is enabled, set callback function.
	int flag;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flag);
	if(flag & GL_CONTEXT_FLAG_DEBUG_BIT) 
	{
		glEnable(GL_DEBUG_OUTPUT);  // Enable debug output
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // Tell opengl to directly call the callback function the moment an error occured.
		glDebugMessageCallback(util::debugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	
	// Viewport transform
	glViewport(0, 0, SRC_WIDTH, util::Globals::camera.height());

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

	// Init font
	CharacterRender* cr = CharacterRender::getInstance();
	cr->init_fonts("./fonts/arial.ttf");

	// Model
	Model model("./model/cubes/cubes.obj");

	// Shaders
	Shader::BlinnPhong bp_shader;
	Shader::PBR pbr_shader;
	Shader::SingleColor color_shader;
	Shader::SkyCube skybox_shader;
	Shader::Normal normal_shader;
	Shader::CascadeMap cascade_shader;
	Shader::DepthCubemap depth_cube_shader;
	Shader::TangentNormal tangent_shader;

	// Lights
	Point_light light1(glm::vec3(2.f, 1.5f, -2.f), glm::vec3(1.f, .4392f, .3294f), 20.0f);
	Point_light light4(glm::vec3(-2.f, 1.5f, 2.f), glm::vec3(0.3137f, 0.8627f, 0.9961f), 20.0f);
	Direction_light light5(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(0.f, 1.8f, 0.f), glm::vec3(.9373f, .702f, .4745f), 2.0f);
	Spot_light light6(glm::vec3(-2.f, -2.6f, -1.5f), glm::vec3(2.25f, 1.91f, 2.27f), glm::vec3(.5f, .5f, .5f), 15.0f);

	// Scene framebuffer for postprocessing
	FBOManager::genWindowFBOs();

	// Initialize jitter vector
	TAAManager::matchJitterVec(util::Globals::camera);

	while(!glfwWindowShouldClose(window))
	{
		// Process input
		util::process_input(window);

		// Start of imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Rendering preperation
		FBOManager::clearBuffers();
		if(!util::Globals::deferred) glEnable(GL_MULTISAMPLE);
		else glDisable(GL_MULTISAMPLE);
		Shader::Shader::updateUniformBlocks(util::Globals::camera);
		// glm::mat4 model_mat(1.f);
		// model_mat = glm::rotate(model_mat, glm::radians((float)glfwGetTime() * 15.f), glm::vec3(0.f, 1.f, 0.f));
		// model.set_model(model_mat);
		model.setCullface(util::Globals::cullFace);
		model.setBlend(util::Globals::blend);

		// Draw depth maps
		model.depthmapPass(cascade_shader, depth_cube_shader);
		// Draw model.If deferred rendering is enabled, we draw on g-buffers, if not we draw objects directly.
		if(util::Globals::deferred)
		{
			model.gbufferPass(util::Globals::pbrMat ? static_cast<Shader::GBuffer>(*Shader::GBufferPBR::getInstance()) : static_cast<Shader::GBuffer>(*Shader::GBufferBP::getInstance()), 
				util::Globals::camera, 
				FBOManager::FBOData::windowsSizeFBOs["GemometryFBO"].fbo);
			PostprocQuad::getInstance()->lightingPass(util::Globals::pbrMat ? static_cast<Shader::LightingPass>(*Shader::LightingPassPBR::getInstance()) : static_cast<Shader::LightingPass>(*Shader::LightingPassBP::getInstance()), 
				util::Globals::camera,
				FBOManager::FBOData::windowsSizeFBOs["sceneFBO"].fbo);
			if(util::Globals::SSAO)
			{
				PostprocQuad::getInstance()->SSAOPass(*Shader::SSAO::getInstance(), 
					*Shader::SSAOBlur::getInstance(), 
					FBOManager::FBOData::windowsSizeFBOs["SSAOFBO"].fbo, 
					FBOManager::FBOData::windowsSizeFBOs["SSAOBlurFBO"].fbo);
			}
			model.transparentPass(*Shader::TransparentWBPBR::getInstance(), util::Globals::camera, FBOManager::FBOData::windowsSizeFBOs["transparentFBO"].fbo);
			PostprocQuad::getInstance()->compositePass(*Shader::Composite::getInstance(), FBOManager::FBOData::windowsSizeFBOs["sceneFBO"].fbo);
			PostprocQuad::getInstance()->TAAResolvePass(*Shader::TAAResolve::getInstance(), TAAManager::historyFBO.fbo);
		}
		else
		{
			model.foward(util::Globals::pbrMat ? (Shader::ObjectShader&)pbr_shader : (Shader::ObjectShader&)bp_shader, 
				util::Globals::camera, 
				FBOManager::FBOData::windowsSizeFBOs["sceneMSFBO"].fbo);
		}

		// Draw skybox
		if(util::Globals::skybox)
			Skybox::getInstance()->draw(skybox_shader, util::Globals::camera, 
				util::Globals::deferred ? FBOManager::FBOData::windowsSizeFBOs["sceneFBO"].fbo : FBOManager::FBOData::windowsSizeFBOs["sceneMSFBO"].fbo);

		// Post process
		PostprocQuad::getInstance()->drawOnScreen(*Shader::PostProc::getInstance());

		// Draw outline
		model.drawOutline(color_shader, util::Globals::camera);

		// Draw light
		Light_vertices::getInstance()->forward(color_shader, util::Globals::camera);
		
		// Draw model normals
		if(util::Globals::visualizeNormal)
			model.drawNormals(normal_shader, util::Globals::camera);

		// Draw model tangent normal
		if(util::Globals::visualizeTangent) 
			model.drawTangent(tangent_shader, util::Globals::camera);

		// Draw frambuffer debuger window
 		FBODebuger::getInstance(SRC_WIDTH, util::Globals::camera.height())
			->draw(*Shader::FBODebuger::getInstance(), Model::getTexture("historyColor").texUnit);

		// some variable updates
		double current_time = glfwGetTime();
		util::Globals::deltaTime = current_time - util::Globals::lastTime;
		util::Globals::lastTime = current_time;
		if(++util::Globals::frameIndexMod16 >= 16) util::Globals::frameIndexMod16 -= 16;
		if(util::Globals::deferred && util::Globals::deferedFrameIndex <= 2) ++util::Globals::deferedFrameIndex;

		// Imgui user interface design
		util::ImGUIDesign(model);

		// End of imgui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		// Check and call events and swap buffers
		glfwPollEvents();  // 检测有无按下按键，鼠标操作等等，并执行相应的回调函数
		glfwSwapBuffers(window);  // 双缓冲，把back buffer换到front buffer去，得以显示渲染图

		// During the end of the current frame
		// Check whether to switch current selected model
		Model::switch_model(model);
	}
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}