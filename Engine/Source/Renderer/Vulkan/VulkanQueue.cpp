#include "EnginePCH.h"
#include "VulkanQueue.h"

#include "VulkanRHI.h"
#include "Renderer/Common/RHIDependency.h"
#include "Renderer/Common/RHIPayload.h"
#include "Renderer/Vulkan/VulkanCommandList.h"

void CVulkanQueue::Submit(FRHIPayload* Payload)
{
    VkCommandBufferSubmitInfo CommandBufferSubmitInfo = {};
    CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    CommandBufferSubmitInfo.commandBuffer = (VkCommandBuffer)Payload->GetCommandList()->GetNativeHandle();

    VkSemaphoreSubmitInfo WaitSemaphoreSubmitInfo = {};
    WaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    WaitSemaphoreSubmitInfo.semaphore = (VkSemaphore)Payload->GetImageAvailableSemaphore()->GetNativeHandle();
    WaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;

    VkSemaphoreSubmitInfo ReadySemaphoreSubmitInfo = {};
    ReadySemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    ReadySemaphoreSubmitInfo.semaphore = (VkSemaphore)Payload->GetRenderFinishedSemaphore()->GetNativeHandle();
    ReadySemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

    VkSubmitInfo2 SubmitInfo = {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    SubmitInfo.waitSemaphoreInfoCount = 1;
    SubmitInfo.pWaitSemaphoreInfos = &WaitSemaphoreSubmitInfo;
    SubmitInfo.signalSemaphoreInfoCount = 1;
    SubmitInfo.pSignalSemaphoreInfos = &ReadySemaphoreSubmitInfo;
    SubmitInfo.commandBufferInfoCount = 1;
    SubmitInfo.pCommandBufferInfos = &CommandBufferSubmitInfo;

    VkResult Result = vkQueueSubmit2(Queue, 1, &SubmitInfo, (VkFence)Payload->GetInFlightFence()->GetNativeHandle());
    RK_ASSERT(Result == VK_SUCCESS, "Failed to submit command buffer to graphics queue.");
}

void CVulkanQueue::WaitIdle()
{
}

ERHIQueueClass CVulkanQueue::GetClass() const
{
    return Class;
}

const FRHIQueueCapabilities & CVulkanQueue::GetCapabilities() const
{
    return Capabilities;
}

void * CVulkanQueue::GetNativeHandle() const
{
    return Queue;
}
