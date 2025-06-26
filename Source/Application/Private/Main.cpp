#include "Core/Public/Engine.h"

int main()
{
    CEngine* Engine = new CEngine();

    Engine->Start();

    Engine->Run();

    Engine->Stop();

    delete Engine;
    
    return 0;
}
