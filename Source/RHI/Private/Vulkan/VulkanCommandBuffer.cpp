#include "RocketPCH.h"
#include "VulkanCommandBuffer.h"

#include "RHI/Public/Vulkan/VulkanRHIMinimal.h"
#include "RHI/Private/Vulkan/VulkanQueue.h"
#include "RHI/Private/Vulkan/VulkanDevice.h"

FVulkanCommandBuffer::FVulkanCommandBuffer(FVulkanCommandBufferPool& InPool)
    : Pool(InPool),
    State(EVulkanCommandBufferState::NotAllocated)
{
    Alloc();
}

FVulkanCommandBuffer::~FVulkanCommandBuffer()
{
    Free();
}

void FVulkanCommandBuffer::Alloc()
{
    assert(State == EVulkanCommandBufferState::NotAllocated);

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
    CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInfo.commandPool = Pool.GetHandle();
    CommandBufferAllocateInfo.commandBufferCount = 1;
    CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkResult Result = vkAllocateCommandBuffers(GetVulkanRHIMinimal()->RHIGetVkDevice(), &CommandBufferAllocateInfo, &Handle);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate command buffers.");

    State = EVulkanCommandBufferState::ReadyForBegin;
}

void FVulkanCommandBuffer::Free()
{
    assert(State != EVulkanCommandBufferState::NotAllocated);

    vkFreeCommandBuffers(GetVulkanRHIMinimal()->RHIGetVkDevice(), Pool.GetHandle(), 1, &Handle);

    State = EVulkanCommandBufferState::NotAllocated;
}

void FVulkanCommandBuffer::Begin()
{
    assert(State == EVulkanCommandBufferState::ReadyForBegin || State == EVulkanCommandBufferState::NeedReset);

    if (State == EVulkanCommandBufferState::NeedReset)
    {
        Reset();
    }

    VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
    CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CommandBufferBeginInfo.pNext = nullptr;
    CommandBufferBeginInfo.pInheritanceInfo = nullptr;

    VkResult Result = vkBeginCommandBuffer(Handle, &CommandBufferBeginInfo);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to begin recording command buffer.");

    State = EVulkanCommandBufferState::IsInsideBegin;
}

void FVulkanCommandBuffer::End()
{
    assert(State == EVulkanCommandBufferState::IsInsideBegin);

    vkEndCommandBuffer(Handle);

    State = EVulkanCommandBufferState::HasEnded;
}

void FVulkanCommandBuffer::Submit()
{
    assert(State == EVulkanCommandBufferState::HasEnded);

    static const auto Start = std::chrono::steady_clock::now();
    const auto Now = std::chrono::steady_clock::now();

    SubmissionTime = std::chrono::duration<double>(Now - Start).count();

    State = EVulkanCommandBufferState::Submitted;
}

void FVulkanCommandBuffer::Reset()
{
    assert(EVulkanCommandBufferState::NeedReset);

    vkResetCommandBuffer(Handle, 0);

    State = EVulkanCommandBufferState::ReadyForBegin;
}

FVulkanCommandBufferPool::FVulkanCommandBufferPool(FVulkanDevice& InDevice, FVulkanQueue& InQueue)
    : Device(InDevice),
    Queue(InQueue)
{
    Initialize();
}

FVulkanCommandBufferPool::~FVulkanCommandBufferPool()
{
    Shutdown();
}

FVulkanCommandBuffer* FVulkanCommandBufferPool::Acquire()
{
    if (FreeCommandBuffers.empty())
    {
        FVulkanCommandBuffer* CommandBuffer = new FVulkanCommandBuffer(*this);
        return CommandBuffer;
    }

    FVulkanCommandBuffer* CommandBuffer = FreeCommandBuffers.back();
    FreeCommandBuffers.pop_back();
    return CommandBuffer;
}

void FVulkanCommandBufferPool::Recycle(FVulkanCommandBuffer* CommandBuffer)
{
    CommandBuffer->Reset();

    FreeCommandBuffers.push_back(CommandBuffer);
}

void FVulkanCommandBufferPool::Initialize()
{
    VkCommandPoolCreateInfo CommandPoolCreateInfo = {};
    CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CommandPoolCreateInfo.pNext = nullptr;
    CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    CommandPoolCreateInfo.queueFamilyIndex = Queue.GetQueueFamilyIndex();

    VkResult Result = vkCreateCommandPool(Device.GetVkDevice(), &CommandPoolCreateInfo, nullptr, &Handle);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create command pool.");
}

void FVulkanCommandBufferPool::Shutdown()
{
    for (FVulkanCommandBuffer* CommandBuffer : FreeCommandBuffers)
    {
        delete CommandBuffer;
    }
    FreeCommandBuffers.clear();

    vkDestroyCommandPool(Device.GetVkDevice(), Handle, nullptr); 
}
