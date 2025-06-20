#pragma once

#include "RHI/Public/Common/RHI.h"
#include "RHI/Public/Vulkan/VulkanRHIMinimal.h"

class CVulkanRHI : public IVulkanRHIMinimal
{
public:
    // IRHI interface
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void Tick(float DeltaTime) override;

    [[nodiscard]] virtual const char* GetName() const override;
    [[nodiscard]] virtual const char* GetVersion() const override;
    [[nodiscard]] virtual ERHIInterfaceType  GetInterfaceType() const noexcept override;
    [[nodiscard]] virtual IRHI* GetNonValidationRHI() const noexcept override;

    // IVulkanRHIMinimal interface
    virtual VkInstance GetInstance() const override;
    virtual VkDevice GetDevice() const override;
    virtual VkPhysicalDevice GetPhysicalDevice() const override;

    virtual VkQueue GetGraphicsQueue() const override;
    virtual VkQueue GetComputeQueue() const override;
    virtual VkQueue GetCopyQueue() const override;
    virtual VkQueue GetPresentQueue() const override;

private:
    VkInstance Instance;
    VkDevice Device;
    VkPhysicalDevice PhysicalDevice;
};
