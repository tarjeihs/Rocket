#include "RocketPCH.h"
#include "RHI/Public/Vulkan/VulkanRHI.h"

void CVulkanRHI::Init()
{
}

void CVulkanRHI::Shutdown()
{
}

void CVulkanRHI::Tick(float DeltaTime)
{
}

const char* CVulkanRHI::GetName() const
{
    return "Vulkan";
}

const char* CVulkanRHI::GetVersion() const
{
    return "1.4.313";
}

ERHIInterfaceType CVulkanRHI::GetInterfaceType() const noexcept
{
    return StaticType;
}

IRHI* CVulkanRHI::GetNonValidationRHI() const noexcept
{
    return const_cast<CVulkanRHI*>(this);
}

VkInstance CVulkanRHI::GetInstance() const
{
    return Instance;
}

VkDevice CVulkanRHI::GetDevice() const
{
    return Device;
}

VkPhysicalDevice CVulkanRHI::GetPhysicalDevice() const
{
    return PhysicalDevice;
}

VkQueue CVulkanRHI::GetGraphicsQueue() const
{
    return nullptr;
}

VkQueue CVulkanRHI::GetComputeQueue() const
{
    return nullptr;
}

VkQueue CVulkanRHI::GetCopyQueue() const
{
    return nullptr;
}

VkQueue CVulkanRHI::GetPresentQueue() const
{
    return nullptr;
}
