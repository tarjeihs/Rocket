#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

class PVulkanRHI;

class PVulkanDevice
{
public:
    PVulkanDevice(PVulkanRHI* InRHI)
        : RHI(InRHI)
    {
        GPU = nullptr;
        Device = nullptr;
        GraphicsQueue = nullptr;
        PresentQueue = nullptr;
        GraphicsFamily = 0;
        PresentFamily = 0;
    }

    void Init();
    void Shutdown();

    VkPhysicalDevice GetVkPhysicalDevice() const;
    VkDevice GetVkDevice() const;
    VkQueue GetGraphicsQueue() const;
    VkQueue GetPresentQueue() const;
    std::optional<uint32_t> GetGraphicsFamilyIndex() const;
    std::optional<uint32_t> GetPresentFamilyIndex() const;
    const std::vector<VkSurfaceFormatKHR>& GetSurfaceFormats() const;
    const std::vector<VkPresentModeKHR>& GetPresentModes() const;
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities() const;
    VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const;

private:
    PVulkanRHI* RHI;

    VkDevice Device;
    VkPhysicalDevice GPU;

    VkQueue GraphicsQueue;
    VkQueue PresentQueue;

    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

    std::vector<VkSurfaceFormatKHR> SurfaceFormats;
    std::vector<VkPresentModeKHR> PresentModes;
};