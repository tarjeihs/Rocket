#include "Application.h"

#include "Core/Engine.h"

void PApplication::Run()
{
	Engine = new PEngine();
	
	Engine->Start();

	Engine->Run();
}

void PApplication::Exit()
{
	Engine->Stop();
	
	delete Engine;
}
