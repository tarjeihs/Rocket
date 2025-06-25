#pragma once

#include <queue>

class FVulkanDevice;
class FVulkanCommandBufferPayload;
class FVulkanCommandBufferPool;
class FVulkanCommandBufferContext;

class FVulkanQueue
{
private:
    FVulkanDevice& Device;
    VkQueueFlags QueueFlags;

public:
    FVulkanQueue(FVulkanDevice& InDevice, VkQueueFlags InQueueFlags, bool InPresent, uint32_t InQueueFamilyIndex, uint32_t InQueueIndex);
     ~FVulkanQueue();

    void Submit(FVulkanCommandBufferContext& Context);
    void Await();

    inline VkQueue GetHandle() const;
    inline VkQueueFlags GetQueueFlags() const;
    inline bool HasPresent() const;
    inline uint32_t GetQueueFamilyIndex() const;
    inline uint32_t GetQueueIndex() const;

    inline VkSemaphore GetTimelineSemaphore() const;
    inline uint64_t GetTimelineSemaphoreValue() const;

    inline FVulkanCommandBufferPool* GetCommandBufferPool() const;

private:
    void Init();
    void Shutdown();

private:
    VkQueue Handle;
    bool Present;
    uint32_t QueueFamilyIndex;
    uint32_t QueueIndex;

    VkSemaphore TimelineSemaphore;
    uint64_t NextTimelineSemaphoreValue;

    std::unique_ptr<FVulkanCommandBufferPool> CommandBufferPool;
    std::queue<FVulkanCommandBufferPayload*> SubmissionQueue;
};

inline VkQueue FVulkanQueue::GetHandle() const
{
    return Handle;
}

inline VkQueueFlags FVulkanQueue::GetQueueFlags() const
{
    return QueueFlags;
}

inline bool FVulkanQueue::HasPresent() const
{
    return Present;
}

inline uint32_t FVulkanQueue::GetQueueFamilyIndex() const
{
    return QueueFamilyIndex;
}

inline uint32_t FVulkanQueue::GetQueueIndex() const
{
    return QueueIndex;
}

uint64_t FVulkanQueue::GetTimelineSemaphoreValue() const
{
    return NextTimelineSemaphoreValue;
}

VkSemaphore FVulkanQueue::GetTimelineSemaphore() const
{
    return TimelineSemaphore;
}

inline FVulkanCommandBufferPool* FVulkanQueue::GetCommandBufferPool() const
{
    return CommandBufferPool.get();
}