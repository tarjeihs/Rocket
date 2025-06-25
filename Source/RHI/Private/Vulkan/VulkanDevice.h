#pragma once

#include <vector>

class FVulkanQueue;

class FVulkanDevice
{
private:
    VkPhysicalDevice PhysicalDevice;

public:
    FVulkanDevice(VkPhysicalDevice InPhysicalDevice);
    ~FVulkanDevice();

    void WaitUntilIdle() const;

    FVulkanQueue* GetQueue(VkQueueFlags QueueFlags) const;
    FVulkanQueue* GetGraphicsQueue() const;
    FVulkanQueue* GetComputeQueue() const;
    FVulkanQueue* GetTransferQueue() const;
    FVulkanQueue* GetPresentQueue() const;

    inline VkDevice GetVkDevice() const;
    inline VkPhysicalDevice GetVkPhysicalDevice() const;
    inline VmaAllocator GetVmaAllocator() const;
    inline VkDebugUtilsMessengerEXT GetVkDebugUtilsMessenger() const;

private:
    void Initialize();
    void Shutdown();

private:
    VkDevice Device;
    VmaAllocator Allocator;
    VkDebugUtilsMessengerEXT DebugUtilsMessenger;
    std::vector<std::unique_ptr<FVulkanQueue>> Queues;
};

inline VkDevice FVulkanDevice::GetVkDevice() const
{
    return Device;
}

inline VkPhysicalDevice FVulkanDevice::GetVkPhysicalDevice() const
{
    return PhysicalDevice;
}

inline VmaAllocator FVulkanDevice::GetVmaAllocator() const
{
    return Allocator;
}

inline VkDebugUtilsMessengerEXT FVulkanDevice::GetVkDebugUtilsMessenger() const
{
    return DebugUtilsMessenger;
}
