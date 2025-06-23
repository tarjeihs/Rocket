#pragma once

class FVulkanCommandBufferPool;
class FVulkanQueue;
class FVulkanDevice;

enum class EVulkanCommandBufferType : uint8_t
{
    Primary,
    Secondary,
};

enum class EVulkanCommandBufferState : uint8_t
{
    NotAllocated,
    ReadyForBegin,
    IsInsideBegin,
    HasEnded,
    Submitted,
    NeedReset
};

class FVulkanCommandBuffer
{
private:
    FVulkanCommandBufferPool& Pool;

public:
    FVulkanCommandBuffer(FVulkanCommandBufferPool& InPool);
    ~FVulkanCommandBuffer();

    void Begin();
    void End();

    void Submit();
    void Reset();

    inline EVulkanCommandBufferState GetState() const;
    inline VkCommandBuffer GetHandle() const;
    inline double GetSubmissionTime() const;

private:
    void Alloc();
    void Free();

private:
    EVulkanCommandBufferState State;
    VkCommandBuffer Handle;
    double SubmissionTime;
};

class FVulkanCommandBufferPool
{
private:
    FVulkanDevice& Device;
    FVulkanQueue& Queue;

public:
    FVulkanCommandBufferPool(FVulkanDevice& InDevice, FVulkanQueue& InQueue);
    ~FVulkanCommandBufferPool();

    FVulkanCommandBuffer* Acquire();
    void Recycle(FVulkanCommandBuffer* CommandBuffer);

    inline VkCommandPool GetHandle() const;

private:
    void Initialize();
    void Shutdown();

private:
    VkCommandPool Handle;
    std::vector<FVulkanCommandBuffer*> FreeCommandBuffers;
};

EVulkanCommandBufferState FVulkanCommandBuffer::GetState() const
{
    return State;
}

VkCommandBuffer FVulkanCommandBuffer::GetHandle() const
{
    return Handle;
}

double FVulkanCommandBuffer::GetSubmissionTime() const
{
    return SubmissionTime;
}

VkCommandPool FVulkanCommandBufferPool::GetHandle() const
{
    return Handle;
}
