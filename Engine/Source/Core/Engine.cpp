#include "EnginePCH.h"
#include "Engine.h"

#include "Core/Subsystem.h"
#include "Platform/Generic/GenericWindow.h"
#include "Renderer/VulkanRHI.h"
#include "Utils/Profiler.h"

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
		PROFILE_FUNC_SCOPE("PEngine::Run")

		Timestep.Reset();

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

	PProfiler::Flush();

	GEngine = nullptr;
}
