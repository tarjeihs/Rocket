#include "Engine.h"

int main()
{
    CEngine* Engine = new CEngine();

    Engine->Start();

    Engine->Run();

    Engine->Stop();
    
    return 0;
}
