#include "RocketPCH.h"
#include "Core/Public/Engine.h"

#include "IO/Public/GLFWWindow.h"

CEngine* CEngine::GEngine = nullptr;

void CEngine::Start()
{
	GEngine = this;

	FLogger::Init();

	SWindowSpecification WindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };
	
	Window = new PGenericWindow(WindowSpecification);
	Scene = new PScene();

	Window->CreateNativeWindow();
	Renderer->Init();
	Scene->Init();
}

void CEngine::Run()
{
	while (!Window->ShouldClose())
	{
		Window->Poll();

		Renderer->Render();
	}
}

void CEngine::Stop()
{
	Renderer->Shutdown();
	Scene->Cleanup();
	Window->DestroyNativeWindow();

	delete Scene;
	delete Renderer;
	delete Window;

	GEngine = nullptr;
}
