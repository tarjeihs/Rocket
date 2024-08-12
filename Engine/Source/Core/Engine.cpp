#include "Engine.h"

#include "Scene/Scene.h"

#include "Core/Logger.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Vulkan/VulkanRenderer.h"

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
		Window->Poll();
		Window->Swap();

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
