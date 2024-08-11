#include "Engine.h"

#include "Scene/Scene.h"
#include "Core/Window.h"
#include "Renderer/Renderer.h"

#include "Platform/Windows/WindowsWindow.h"

PEngine* PEngine::GEngine = nullptr;

void PEngine::Start()
{
	GEngine = this;

	Scene = new PScene();
	Window = new PWindowsWindow(SWindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT } );
	Renderer = new IRenderer();

	Window->CreateNativeWindow();
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

	delete Scene;
	delete Window;
	delete Renderer;

	GEngine = nullptr;
}
