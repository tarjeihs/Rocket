#include "RocketPCH.h"
#include "RHI/Public/Common/RHI.h"

IRHI* GRHI = nullptr;
IRHIModule* GRHIModule = nullptr;

void SetRHIModule(ERHIInterfaceType InInterfaceType)
{
    if (GRHI)
    {
        GRHI->Shutdown();
        delete GRHI;
        GRHI = nullptr;
    }

    if (GRHIModule)
    {
        delete GRHIModule;
        GRHIModule = nullptr;
    }

    switch (InInterfaceType)
    {
        case ERHIInterfaceType::Vulkan: GRHIModule = CreateVulkanRHIModule(); break;
    }

    GRHI = GRHIModule->CreateRHI();
}