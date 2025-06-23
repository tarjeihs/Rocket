#pragma once

#include "RHI/Public/Vulkan/VulkanRHIMinimal.h"

class FVulkanViewport;
class FVulkanDevice;
class FVulkanCommandBufferContext;

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

    virtual void RHIWaitUntilIdle() const override;

    // IVulkanRHIMinimal interface
    virtual VkInstance RHIGetVkInstance() const override;

    virtual VkDevice RHIGetVkDevice() const override;
    virtual VkPhysicalDevice RHIGetVkPhysicalDevice() const override;

    virtual VkQueue RHIGetGraphicsVkQueue() const override;
    virtual VkQueue RHIGetComputeVkQueue() const override;
    virtual VkQueue RHIGetTransferVkQueue() const override;
    virtual VkQueue RHIGetPresentVkQueue() const override;

    virtual uint32_t RHIGetGraphicsQueueFamilyIndex() const override;
    virtual uint32_t RHIGetComputeQueueFamilyIndex() const override;
    virtual uint32_t RHIGetTransferQueueFamilyIndex() const override;
    virtual uint32_t RHIGetPresentQueueFamilyIndex() const override;

    virtual VkSurfaceKHR RHIGetVkSurface() const override;
    virtual VkSwapchainKHR RHIGetVkSwapchain() const override;
    virtual VkExtent2D RHIGetSwapchainExtent() const override;
    virtual VkPresentModeKHR RHIGetSwapchainPresentMode() const override;
    virtual VkSurfaceFormatKHR RHIGetSwapchainSurfaceFormat() const override;

    virtual std::span<const char*> GetValidationExtensions() override;
    virtual std::span<const char*> GetInstanceExtensions() override;
    virtual std::span<const char*> GetPhysicalDeviceExtensions() override;

    // CVulkanRHI interface
    inline FVulkanDevice* GetDevice() const;
    inline FVulkanViewport* GetViewport() const;

private:
    VkInstance Instance;

    FVulkanDevice* Device;
    FVulkanViewport* Viewport;

    std::vector<const char*> InstanceExtensions;
    std::vector<const char*> ValidationLayerExtensions;
    std::vector<const char*> PhysicalDeviceExtensions;
};
