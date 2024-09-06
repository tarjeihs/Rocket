#pragma once

#include "Core/Engine.h"
#include "Renderer/Common/Renderer.h"

class IRHI 
{
public:
    virtual ~IRHI() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Resize() = 0;
    virtual void Render() = 0;
};

#define RK_RHI VULKAN

#if RK_RHI == VULKAN
    using RK_RHI_TYPE = class PVulkanRHI;
#elif RK_RHI == D3D12
    using RK_RHI_TYPE = class PD3D12RHI;
#elif RK_RHI == METAL
    using RK_RHI_TYPE = class PMetalRHI;
#endif

template<typename TRHI = RK_RHI_TYPE>
inline TRHI* GetRHI()
{
    IRHI* RHI = GetEngine()->GetRHI();
    return static_cast<TRHI*>(RHI);
}