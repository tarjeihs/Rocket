#pragma once

extern PApplication* CreateApplication();

int main()
{
    auto Application = CreateApplication();

    Application->Run();

    delete Application;

    return 0;
}