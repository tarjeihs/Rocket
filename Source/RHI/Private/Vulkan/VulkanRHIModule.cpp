#include "RocketPCH.h"
#include "RHI/Public/Vulkan/VulkanRHIModule.h"

#include "RHI/Public/Vulkan/VulkanRHI.h"

IRHI* FVulkanRHIModule::CreateRHI()
{
    return new CVulkanRHI();
}

IRHIModule* CreateVulkanRHIModule()
{
    return new FVulkanRHIModule();
}