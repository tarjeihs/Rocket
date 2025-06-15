#pragma once

#include "Renderer/Common/RHIDevice.h"

class CVulkanViewport;

class CVulkanDevice : public IRHIDevice
{
public:
    // IRHIDevice interface
    virtual void* GetNativeInstance() const final override;
    virtual void WaitUntilIdle() const final override;

    void CreateInstance();
    void CreateDevice(CVulkanViewport* Viewport);

    void FreeInstance();
    void FreeDevice();

    bool GetGraphicsQueueFamily(VkPhysicalDevice InPhysicalDevice, VkSurfaceKHR Surface, std::optional<uint32_t>& OutGraphicsQueueFamily);
    bool GetPresentQueueFamily(VkPhysicalDevice InPhysicalDevice, VkSurfaceKHR Surface, std::optional<uint32_t>& OutPresentQueueFamily);

    void GetGraphicsQueue(VkSurfaceKHR Surface, VkQueue& OutGraphicsQueue);
    void GetPresentQueue(VkSurfaceKHR Surface, VkQueue& OutPresentQueue);

    VkInstance Instance;
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;
    VkDebugUtilsMessengerEXT DebugCallback;
    VmaAllocator Allocator;

    std::vector<const char*> ValidationLayerExtensions;
    std::vector<const char*> InstanceExtensions;
    std::vector<const char*> PhysicalDeviceExtensions;
};