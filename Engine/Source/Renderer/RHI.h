#pragma once

#include "Core/Engine.h"

class IRHI 
{
public:
    virtual ~IRHI() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Resize() = 0;
    virtual void Render() = 0;
};

template<typename TRHI = IRHI>
inline TRHI* GetRHI()
{
    IRHI* RHI = PEngine::Get()->GetRHI();
    return static_cast<TRHI*>(RHI);
}