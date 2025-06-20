#include "RocketPCH.h"
#include "Core/Public/Engine.h"

#include "IO/Public/GLFWWindow.h"
#include "RHI/Public/Common/RHI.h"

CEngine* CEngine::GEngine = nullptr;

ERHIType RHIType = ERHIType::Vulkan;

void CEngine::Start()
{
	GEngine = this;

	FLogger::Init();

	SWindowSpecification WindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };
    
    IRHIModule* RHIModule = nullptr;

    switch (RHIType)
    {
        case ERHIType::Vulkan: RHIModule = CreateVulkanRHIModule(); break;
    }
    GRHI = RHIModule->CreateRHI();
	
	Window = new PGenericWindow(WindowSpecification);
	Scene = new PScene();

	//Window->CreateNativeWindow();
    GRHI->Init();
	Scene->Init();
}

void CEngine::Run()
{
	while (!Window->ShouldClose())
	{
		Window->Poll();

        GRHI->Render();
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
