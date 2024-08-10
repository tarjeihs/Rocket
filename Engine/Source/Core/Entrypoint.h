#pragma once

#ifdef RK_PLATFORM_WINDOWS

extern PApplication* CreateApplication();

int main()
{
    auto Application = CreateApplication();

    Application->Run();

    delete Application;

    return 0;
}

#endif