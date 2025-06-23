#pragma once

#include <queue>

class FVulkanDevice;
class FVulkanCommandBufferPayload;
class FVulkanCommandBufferPool;
class FVulkanCommandBufferContext;

//enum class EVulkanQueueFamily : uint8_t
//{
//    Undefined = 0,
//    Graphics = 1 << 0,
//    Compute = 1 << 1,
//    Transfer = 1 << 2,
//    Sparse = 1 << 3
//};
//
//inline EVulkanQueueFamily operator|(EVulkanQueueFamily lhs, EVulkanQueueFamily rhs) { return static_cast<EVulkanQueueFamily>(static_cast<std::underlying_type_t<EVulkanQueueFamily>>(lhs) | static_cast<std::underlying_type_t<EVulkanQueueFamily>>(rhs)); }
//inline EVulkanQueueFamily operator&(EVulkanQueueFamily lhs, EVulkanQueueFamily rhs) { return static_cast<EVulkanQueueFamily>(static_cast<std::underlying_type_t<EVulkanQueueFamily>>(lhs) & static_cast<std::underlying_type_t<EVulkanQueueFamily>>(rhs) ); }
//inline EVulkanQueueFamily operator~(EVulkanQueueFamily value) { return static_cast<EVulkanQueueFamily>(~static_cast<std::underlying_type_t<EVulkanQueueFamily>>(value)); }
//
//inline EVulkanQueueFamily& operator|=(EVulkanQueueFamily& lhs, EVulkanQueueFamily rhs) { lhs = lhs | rhs; return lhs; }
//inline EVulkanQueueFamily& operator&=(EVulkanQueueFamily& lhs, EVulkanQueueFamily rhs) { lhs = lhs & rhs; return lhs; }
//inline EVulkanQueueFamily& operator^=(EVulkanQueueFamily& lhs, EVulkanQueueFamily rhs) { lhs = lhs ^ rhs; return lhs; }

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
    VkQueue Handle;
    bool Present;
    uint32_t QueueFamilyIndex;
    uint32_t QueueIndex;

    VkSemaphore TimelineSemaphore;
    uint64_t NextTimelineSemaphoreValue;

    FVulkanCommandBufferPool* CommandBufferPool;
    std::queue<FVulkanCommandBufferPayload*> CommandBufferPayloads;
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
    return CommandBufferPool;
}