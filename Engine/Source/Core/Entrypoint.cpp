#include "Engine.h"

int main()
{
    PEngine* Engine = new PEngine();

    Engine->Start();

    Engine->Run();

    Engine->Stop();
    
    return 0;
}
