#pragma once

#include "Core/Delegate.h"

class POverlay
{
public:
    PDelegate<> OnRender;

    virtual ~POverlay() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
};

// Overlay is declared as a global pointer
extern POverlay* GOverlay;