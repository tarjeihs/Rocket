#include "RocketPCH.h"
#include "Core/Public/Engine.h"

#include "IO/Public/GLFWWindow.h"
#include "RHI/Public/Common/RHI.h"

CEngine* CEngine::GEngine = nullptr;

ERHIInterfaceType RHIType = ERHIInterfaceType::Vulkan;

void CEngine::Start()
{
	GEngine = this;

	FLogger::Init();

	SWindowSpecification WindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };
	
	Window = new PGenericWindow(WindowSpecification);
	Scene = new PScene();

	SetRHIModule(ERHIInterfaceType::Vulkan);

	Window->CreateNativeWindow();
    GRHI->Init();
	Scene->Init();
}

void CEngine::Run()
{
	while (!Window->ShouldClose())
	{
		//Window->Poll();

        GRHI->Tick(0.0f);
	}
}

void CEngine::Stop()
{
    GRHI->Shutdown();
	Scene->Cleanup();
	Window->DestroyNativeWindow();

	delete Scene;
	delete Window;

	GEngine = nullptr;
}
