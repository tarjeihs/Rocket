#pragma once

#include "VulkanQueue.h"
#include "Renderer/Common/RHIDevice.h"

class CVulkanDevice : public IRHIDevice
{
public:
    // IRHIDevice interface
    virtual void* GetNativeDevice() const final override;
    virtual void* GetNativeDriver() const final override;
    virtual void* GetNativeInstance() const final override;
    virtual void WaitUntilIdle() const final override;

    void Initialize();
    void Shutdown();

    virtual IRHIQueue *GetGraphicsQueue() override;
    virtual IRHIQueue *GetComputeQueue() override;
    virtual IRHIQueue *GetTransferQueue() override;
    virtual IRHIQueue *GetPresentQueue() override;

    VkInstance GetRHIVulkanInstance() const;
    VkPhysicalDevice GetRHIVulkanPhysicalDevice() const;
    VkDevice GetLogicalDevice() const;
    VmaAllocator GetAllocator() const;

private:
    VkInstance Instance;
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;
    VkDebugUtilsMessengerEXT DebugCallback;
    VmaAllocator Allocator;

    std::vector<const char*> ValidationLayerExtensions;
    std::vector<const char*> InstanceExtensions;
    std::vector<const char*> PhysicalDeviceExtensions;

    std::unordered_map<ERHIQueueClass, CVulkanQueue> Queue;
};