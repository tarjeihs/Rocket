#pragma once

#include "Core/Engine.h"

class IRHIDevice;
class IRHIViewport;
class IRHICommandList;

class IRHI
{
public:
    virtual ~IRHI() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Resize() = 0;
    virtual void Render() = 0;

    virtual IRHIDevice* GetDevice() = 0;
    virtual IRHIViewport* GetViewport(uint32_t Index = 0) = 0;
};

#define RK_RHI VULKAN

#if RK_RHI == VULKAN
    using RK_RHI_TYPE = class CVulkanRHI;
#elif RK_RHI == D3D12
    using RK_RHI_TYPE = class CD3D12RHI;
#elif RK_RHI == METAL
    using RK_RHI_TYPE = class CMetalRHI;
#endif

template<typename TRHI = RK_RHI_TYPE>
TRHI* GetRHI()
{
    IRHI* Renderer = GetEngine()->GetRHI();
    return static_cast<TRHI*>(Renderer);
}

#define FRAME_IN_FLIGHT  2
#define DEBUG_VALIDATION 1