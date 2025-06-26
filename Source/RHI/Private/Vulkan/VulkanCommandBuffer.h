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
    void Finish();
    void Reset();

    inline EVulkanCommandBufferState GetState() const;
    inline VkCommandBuffer GetHandle() const;
    inline double GetSubmissionTime() const;

private:
    void Alloc();
    void Free();

    inline uint32_t GetPoolID() const;
    inline void SetPoolID(const uint32_t InPoolID);

private:
    EVulkanCommandBufferState State;
    VkCommandBuffer Handle;
    double SubmissionTime;
    uint32_t PoolID;

    friend class FVulkanCommandBufferPool;
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
    std::vector<FVulkanCommandBuffer*> UsedCommandBuffers;
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

inline uint32_t FVulkanCommandBuffer::GetPoolID() const
{
    return PoolID;
}

inline void FVulkanCommandBuffer::SetPoolID(const uint32_t InPoolID)
{
    PoolID = InPoolID;
}

VkCommandPool FVulkanCommandBufferPool::GetHandle() const
{
    return Handle;
}
