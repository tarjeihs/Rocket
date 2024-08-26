#include "Engine.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>

#include "Core/Assert.h"
#include "Core/Scene.h"
#include "Core/Logger.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Renderer/VulkanRHI.h"

PEngine* PEngine::GEngine = nullptr;

void PEngine::Start()
{
	GEngine = this;

	PLogger::Init();

	SWindowSpecification WindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };
	
	Scene = new PScene();
	RHI = new PVulkanRHI();
	Window = new PWindowsWindow(WindowSpecification);

	Scene->Init();
	Window->CreateNativeWindow();
	RHI->Init();
}

void PEngine::Run()
{
	while (!Window->ShouldClose())
	{
		Timestep.Validate();

		Window->Poll();
		
		float CameraDelta = 10.0f;  // Adjust speed as necessary
		PCamera* Camera = GetScene()->ActiveCamera;
		static glm::vec3 pos = glm::vec3(0.0f, 0.0f, 2.5f);
		static glm::vec3 rot = glm::vec3(0.0f);
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_W) == GLFW_PRESS) { pos += glm::vec3(1.0f, 0.0f, 0.0f) * Timestep.GetDeltaTime(); }
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_S) == GLFW_PRESS) { pos -= glm::vec3(1.0f, 0.0f, 0.0f) * Timestep.GetDeltaTime(); }
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_D) == GLFW_PRESS) { pos += glm::vec3(0.0f, 1.0f, 0.0f) * Timestep.GetDeltaTime(); }
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_A) == GLFW_PRESS) { pos -= glm::vec3(0.0f, 1.0f, 0.0f) * Timestep.GetDeltaTime(); }
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_SPACE) == GLFW_PRESS) { pos += glm::vec3(0.0f, 0.0f, 1.0f) * Timestep.GetDeltaTime(); }
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) { pos -= glm::vec3(0.0f, 0.0f, 1.0f) * Timestep.GetDeltaTime(); }
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_UP) == GLFW_PRESS) { rot += glm::vec3(5.f, 0.0f, 0.0f) * Timestep.GetDeltaTime(); }
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_DOWN) == GLFW_PRESS) { rot -= glm::vec3(5.f, 0.0f, 0.0f) * Timestep.GetDeltaTime(); }
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) { rot += glm::vec3(0.0f, 5.0f, 0.0f) * Timestep.GetDeltaTime(); }
		if (glfwGetKey((GLFWwindow*)Window->GetNativeWindow(), GLFW_KEY_LEFT) == GLFW_PRESS) { rot -= glm::vec3(.0f, 5.0f, 0.0f) * Timestep.GetDeltaTime(); }
		Camera->SetPerspectiveProjection(glm::radians(66.0f), Window->GetAspectRatio(), 0.1f, 1000.0f);
		Camera->CalculateViewMatrix(pos, rot);

		//Renderer->Draw();
		RHI->Render();
	}
}

void PEngine::Stop()
{
	Window->DestroyNativeWindow();
	RHI->Shutdown();

	delete Scene;
	delete Window;

	GEngine = nullptr;
}
