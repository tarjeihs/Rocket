#include "Engine.h"

#include "Scene/Scene.h"

#include "Core/Logger.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Vulkan/VulkanRenderer.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

PEngine* PEngine::GEngine = nullptr;

void PEngine::Start()
{
	GEngine = this;

	PLogger::Init();

	Scene = new PScene();
	Window = new PWindowsWindow(SWindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT } );
	Renderer = new PVulkanRenderer();

	Window->CreateNativeWindow();
	Renderer->Initialize();
}

void PEngine::Run()
{
	while (!Window->ShouldClose())
	{
		Timestep.Validate();

		Window->Poll();
		Window->Swap();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Metrics");
		ImGui::Text("Frame Rate: %f", 1.0f / Timestep.GetDeltaTime());
		ImGui::Text("Frame Time: %fms", Timestep.GetDeltaTime());
		ImGui::Text("Engine Time: %fs", Timestep.GetElapsedTime());
		ImGui::End();
		ImGui::Render();
		
		Renderer->Draw();
	}
}

void PEngine::Stop()
{
	Window->DestroyNativeWindow();
	Renderer->DestroyRenderer();

	delete Scene;
	delete Window;
	delete Renderer;

	GEngine = nullptr;
}
