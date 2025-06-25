#include "RocketPCH.h"
#include "RHI/Private/Vulkan/VulkanQueue.h"

#include "RHI/Private/Vulkan/VulkanCommandBuffer.h"
#include "RHI/Private/Vulkan/VulkanCommandBufferContext.h"
#include "RHI/Public/Vulkan/VulkanRHIMinimal.h"
#include "RHI/Private/Vulkan/VulkanDevice.h"

FVulkanQueue::FVulkanQueue(FVulkanDevice& InDevice, VkQueueFlags InQueueFlags, bool InPresent, uint32_t InQueueFamilyIndex, uint32_t InQueueIndex)
    : Device(InDevice),
    QueueFlags(InQueueFlags),
    Present(InPresent), 
    QueueFamilyIndex(InQueueFamilyIndex),
    QueueIndex(InQueueIndex),
    NextTimelineSemaphoreValue(1)
{
    Init();
}

FVulkanQueue::~FVulkanQueue()
{
    Shutdown();
}

void FVulkanQueue::Submit(FVulkanCommandBufferContext& Context)
{
    std::vector<FVulkanCommandBufferPayload*> Payloads;
    Context.Finalize(Payloads);

    for (FVulkanCommandBufferPayload* Payload : Payloads)
    {
        std::vector<VkSemaphoreSubmitInfo> WaitSemaphoreSubmitInfos;
        for (VkSemaphore Semaphore : Payload->WaitSemaphores)
        {
            VkSemaphoreSubmitInfo WaitSemaphoreSubmitInfo = {};
            WaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            WaitSemaphoreSubmitInfo.semaphore = Semaphore;
            WaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
            WaitSemaphoreSubmitInfos.push_back(WaitSemaphoreSubmitInfo);
        }

        std::vector<VkSemaphoreSubmitInfo> SignalSemaphoreSubmitInfos;
        for (VkSemaphore Semaphore : Payload->SignalSemaphores)
        {
            VkSemaphoreSubmitInfo ReadySemaphoreSubmitInfo = {};
            ReadySemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            ReadySemaphoreSubmitInfo.semaphore = Semaphore;
            ReadySemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
            SignalSemaphoreSubmitInfos.push_back(ReadySemaphoreSubmitInfo);
        }

        std::vector<VkCommandBufferSubmitInfo> CommandBufferSubmitInfos;
        for (FVulkanCommandBuffer* CommandBuffer : Payload->CommandBuffers)
        {
            VkCommandBufferSubmitInfo CommandBufferSubmitInfo = {};
            CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            CommandBufferSubmitInfo.commandBuffer = CommandBuffer->GetHandle();
            CommandBufferSubmitInfos.push_back(CommandBufferSubmitInfo);
            CommandBuffer->Submit();
        }
        
        uint64_t TimelineValue = NextTimelineSemaphoreValue++;
        Payload->TimelineSemaphoreValue = TimelineValue;

        VkSemaphoreSubmitInfo TimelineSemaphoreSubmitInfo = {};
        TimelineSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        TimelineSemaphoreSubmitInfo.semaphore = TimelineSemaphore;
        TimelineSemaphoreSubmitInfo.value     = TimelineValue;
        TimelineSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;
        SignalSemaphoreSubmitInfos.push_back(TimelineSemaphoreSubmitInfo);

        VkSubmitInfo2 SubmitInfo = {};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        SubmitInfo.waitSemaphoreInfoCount = WaitSemaphoreSubmitInfos.size();
        SubmitInfo.pWaitSemaphoreInfos = WaitSemaphoreSubmitInfos.data();
        SubmitInfo.signalSemaphoreInfoCount = SignalSemaphoreSubmitInfos.size();
        SubmitInfo.pSignalSemaphoreInfos = SignalSemaphoreSubmitInfos.data();
        SubmitInfo.commandBufferInfoCount = CommandBufferSubmitInfos.size();
        SubmitInfo.pCommandBufferInfos = CommandBufferSubmitInfos.data();

        VkResult Result = vkQueueSubmit2(Handle, 1, &SubmitInfo, nullptr);
        RK_ASSERT(Result == VK_SUCCESS, "Failed to submit command buffer to graphics queue.");

        SubmissionQueue.push(Payload);
    }
}

void FVulkanQueue::Await()
{
    if (SubmissionQueue.empty())
    {
        return;
    }

    FVulkanCommandBufferPayload* Payload = SubmissionQueue.front();
    SubmissionQueue.pop();

    // At most two command-buffer submissions overlap (keeps latency down).

    VkSemaphoreWaitInfo SemaphoreWaitInfo = {};
    SemaphoreWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    SemaphoreWaitInfo.semaphoreCount = 1;
    SemaphoreWaitInfo.pSemaphores    = &TimelineSemaphore;
    SemaphoreWaitInfo.pValues        = &Payload->TimelineSemaphoreValue;
    vkWaitSemaphores(GetVulkanRHIMinimal()->RHIGetVkDevice(), &SemaphoreWaitInfo, UINT64_MAX);

    for (FVulkanCommandBuffer* CB : Payload->CommandBuffers)
    {
        CB->Reset();
    }

    delete Payload;
}

void FVulkanQueue::Init()
{
    vkGetDeviceQueue(Device.GetVkDevice(), QueueFamilyIndex, 0, &Handle);

    VkSemaphoreTypeCreateInfo TimelineSemaphoreCreateInfo = {};
    TimelineSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    TimelineSemaphoreCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    TimelineSemaphoreCreateInfo.initialValue  = 0;

    VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    SemaphoreCreateInfo.pNext = &TimelineSemaphoreCreateInfo;
    vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &TimelineSemaphore);

    CommandBufferPool = std::make_unique<FVulkanCommandBufferPool>(Device, *this);
}

void FVulkanQueue::Shutdown()
{
    while (!SubmissionQueue.empty())
    {
        FVulkanCommandBufferPayload* Payload = SubmissionQueue.front();
        SubmissionQueue.pop();
        delete Payload;
    }

    CommandBufferPool = nullptr;
}
