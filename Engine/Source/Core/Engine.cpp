#include "Engine.h"

#include "Scene/Scene.h"
#include "Core/Window.h"
#include "Renderer/Renderer.h"

PEngine* PEngine::GEngine = nullptr;

void PEngine::Start()
{
	GEngine = this;

	Scene = new PScene();
	Window = new IWindow();
	Renderer = new IRenderer();
}

void PEngine::Run()
{
	while (!Window->ShouldClose())
	{

	}
}

void PEngine::Stop()
{
	delete Scene;
	delete Window;
	delete Renderer;

	GEngine = nullptr;
}
