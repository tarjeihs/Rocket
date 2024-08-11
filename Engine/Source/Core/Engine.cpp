#include "Engine.h"

#include "Scene/Scene.h"

#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Vulkan/VulkanRenderHardwareInterface.h"

PEngine* PEngine::GEngine = nullptr;

void PEngine::Start()
{
	GEngine = this;

	Scene = new PScene();
	Window = new PWindowsWindow(SWindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT } );
	RenderHardwareInterface = new PVulkanRenderHardwareInterface();

	Window->CreateNativeWindow();
	RenderHardwareInterface->CreateRHI();
}

void PEngine::Run()
{
	while (!Window->ShouldClose())
	{
		Window->Poll();
		Window->Swap();
	}
}

void PEngine::Stop()
{
	Window->DestroyNativeWindow();
	RenderHardwareInterface->DestroyRHI();

	delete Scene;
	delete Window;
	delete RenderHardwareInterface;

	GEngine = nullptr;
}
