#pragma once

class FVulkanDevice;
struct FVulkanCommandBuffer;
struct FVulkanCommandBufferPool;

struct FVulkanCommandBufferPayload
{
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkSemaphore> SignalSemaphores;
    std::vector<FVulkanCommandBuffer*> CommandBuffers;
    uint64_t TimelineSemaphoreValue;
};

// Wait -> Execute -> Signal is the allowed forward transition.
// Attempting to regress state (Execute -> Wait) triggers payload completion and starts new payload.
enum class EVulkanCommandBufferContextState 
{ 
    Wait, 
    Execute, 
    Signal 
};

inline bool operator<(EVulkanCommandBufferContextState a, EVulkanCommandBufferContextState b)
{
    return static_cast<int>(a) < static_cast<int>(b);   
}

class FVulkanCommandBufferContext 
{
private:
    FVulkanCommandBufferPool& Pool;

public:
    FVulkanCommandBufferContext(FVulkanCommandBufferPool& InPool);

    FVulkanCommandBuffer* GetCurrentCommandBuffer();
    FVulkanCommandBufferPool* GetCommandBufferPool();
    FVulkanCommandBufferPayload& GetPayload(EVulkanCommandBufferContextState State); 

    void NewPayload();
    void EndPayload();
    void Finalize(std::vector<FVulkanCommandBufferPayload*>& OutPayloads);

    void AddWaitSemaphore(VkSemaphore WaitSemaphore);
    void AddWaitSemaphore(std::span<VkSemaphore> WaitSemaphores);
    void AddSignalSemaphore(VkSemaphore ReadySemaphore);
    void AddSignalSemaphore(std::span<VkSemaphore> ReadySemaphores);

private:
    std::vector<FVulkanCommandBufferPayload*> Payloads;
    EVulkanCommandBufferContextState CurrentState;
    FVulkanCommandBufferPayload* CurrentPayload;
};
