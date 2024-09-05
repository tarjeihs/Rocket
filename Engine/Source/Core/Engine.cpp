#include "Engine.h"

#include <GLFW/glfw3.h>

#include "Core/Logger.h"
#include "Scene/Scene.h"
#include "Core/Subsystem.h"
#include "Platform/Generic/GenericWindow.h"
#include "Renderer/VulkanRHI.h"

PEngine* PEngine::GEngine = nullptr;

void PEngine::Start()
{
	GEngine = this;

	PLogger::Init();

	SWindowSpecification WindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };
	
	Window = new PGenericWindow(WindowSpecification);
	RHI = new PVulkanRHI();
	Scene = new PScene();

	Window->CreateNativeWindow();
	RHI->Init();
	Scene->Init();

	for (ISubsystem* Subsystem : SSubsystemStaticRegistry::GetStaticRegistry().GetSubsystems())
	{
		Subsystem->OnAttach();
	}
}

void PEngine::Run()
{
	while (!Window->ShouldClose())
	{
		Timestep.Validate();

		Window->Poll();
		
		for (ISubsystem* Subsystem : SSubsystemStaticRegistry::GetStaticRegistry().GetSubsystems())
		{
			Subsystem->OnUpdate(Timestep.GetDeltaTime());
		}

		RHI->Render();
	}
}

void PEngine::Stop()
{
	for (ISubsystem* Subsystem : SSubsystemStaticRegistry::GetStaticRegistry().GetSubsystems())
	{
		Subsystem->OnDetach();
	}

	RHI->Shutdown();
	Scene->Cleanup();
	Window->DestroyNativeWindow();

	delete Scene;
	delete RHI;
	delete Window;

	GEngine = nullptr;
}
