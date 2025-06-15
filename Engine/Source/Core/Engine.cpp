#include "EnginePCH.h"
#include "Engine.h"

#include "Core/Subsystem.h"
#include "Platform/Generic/GenericWindow.h"
#include "../Renderer/Vulkan/VulkanRHI.h"
#include "Utils/Profiler.h"

CEngine* CEngine::GEngine = nullptr;

void CEngine::Start()
{
	GEngine = this;

	FLogger::Init();

	SWindowSpecification WindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };
	
	Window = new PGenericWindow(WindowSpecification);
	Renderer = new CVulkanRHI();
	Scene = new PScene();

	Window->CreateNativeWindow();
	Renderer->Init();
	Scene->Init();

	for (ISubsystem* Subsystem : SSubsystemStaticRegistry::GetStaticRegistry().GetSubsystems())
	{
		Subsystem->OnStart();
	}
}

void CEngine::Run()
{
	while (!Window->ShouldClose())
	{
		PROFILE_FUNC_SCOPE("PEngine::Run")

		Timestep.Reset();

		Window->Poll();
		
		for (ISubsystem* Subsystem : SSubsystemStaticRegistry::GetStaticRegistry().GetSubsystems())
		{
			Subsystem->OnUpdate(Timestep.GetDeltaTime());
		}

		Renderer->Render();
	}
}

void CEngine::Stop()
{
	for (ISubsystem* Subsystem : SSubsystemStaticRegistry::GetStaticRegistry().GetSubsystems())
	{
		Subsystem->OnDetach();
	}

	Renderer->Shutdown();
	Scene->Cleanup();
	Window->DestroyNativeWindow();

	delete Scene;
	delete Renderer;
	delete Window;

	PProfiler::Flush();

	GEngine = nullptr;
}
