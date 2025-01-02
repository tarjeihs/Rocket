#pragma once

#include "Core/Delegate.h"

class FOverlay
{
public:
    TDelegate<> OnRender;

    virtual ~FOverlay() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Execute() = 0;

};

// ImGui is declared as a global pointer
extern FOverlay* GOverlay;
