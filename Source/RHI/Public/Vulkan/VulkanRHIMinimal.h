#pragma once

#include "RHI/Public/Common/RHI.h"

class IVulkanRHIMinimal : public IRHI
{
public:
    static constexpr ERHIInterfaceType StaticType = ERHIInterfaceType::Vulkan;

    virtual VkInstance GetInstance() const = 0;
    virtual VkDevice GetDevice() const = 0;
    virtual VkPhysicalDevice GetPhysicalDevice() const = 0;

    virtual VkQueue GetGraphicsQueue() const = 0;
    virtual VkQueue GetComputeQueue() const = 0;
    virtual VkQueue GetCopyQueue() const = 0;
    virtual VkQueue GetPresentQueue() const = 0;
};