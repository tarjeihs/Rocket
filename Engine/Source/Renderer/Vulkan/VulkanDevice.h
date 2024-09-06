#pragma once

#include <optional>
#include <vector>

struct VkDevice_T;
struct VkPhysicalDevice_T;
struct VkQueue_T;

typedef struct VkDevice_T* VkDevice;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkQueue_T* VkQueue;

struct VkSurfaceFormatKHR;
struct VkSurfaceCapabilitiesKHR;
struct VkPhysicalDeviceProperties;

enum VkPresentModeKHR;

class PVulkanDevice
{
public:
    PVulkanDevice()
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
    VkDevice Device;
    VkPhysicalDevice GPU;

    VkQueue GraphicsQueue;
    VkQueue PresentQueue;

    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

    std::vector<VkSurfaceFormatKHR> SurfaceFormats;
    std::vector<VkPresentModeKHR> PresentModes;
};