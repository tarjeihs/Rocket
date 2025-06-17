#include "EnginePCH.h"
#include "VulkanCommandList.h"

#include "VulkanDevice.h"
#include "VulkanRHI.h"

void CVulkanCommandList::Enqueue(std::function<void()> &&Command)
{
    CommandList.push_back(std::move(Command));
}

void CVulkanCommandList::Execute()
{
    for (std::function<void()> &Command : CommandList)
    {
        Command();
    }
}

void CVulkanCommandList::Reset()
{
    VkResult Result = vkResetCommandBuffer(CommandBuffer, 0);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to reset command buffer.");
}

void CVulkanCommandList::Begin()
{
    VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
    CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CommandBufferBeginInfo.pNext = nullptr;
    CommandBufferBeginInfo.pInheritanceInfo = nullptr;

    VkResult Result = vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to begin recording command buffer.");
}

void CVulkanCommandList::End()
{
    VkResult Result = vkEndCommandBuffer(CommandBuffer);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to finalize command buffer.");

    CommandList.clear();
}

void CVulkanCommandList::Initialize()
{
    // Command pool used by command buffer for submission to graphics queue.
    VkCommandPoolCreateInfo CommandPoolCreateInfo = {};
    CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CommandPoolCreateInfo.pNext = nullptr;
    CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CommandPoolCreateInfo.queueFamilyIndex = GetRHI()->GetDevice()->GetGraphicsQueue()->GetCapabilities().Engine;

    VkResult Result = vkCreateCommandPool((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), &CommandPoolCreateInfo, nullptr, &CommandPool);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create command pool.");

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
    CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInfo.commandPool = CommandPool;
    CommandBufferAllocateInfo.commandBufferCount = 1;
    CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    Result = vkAllocateCommandBuffers((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), &CommandBufferAllocateInfo, &CommandBuffer);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate command buffers.");
}

void CVulkanCommandList::Shutdown()
{
    vkFreeCommandBuffers((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), CommandPool, 1, &CommandBuffer);
    vkDestroyCommandPool((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), CommandPool, nullptr);

    CommandPool = VK_NULL_HANDLE;
    CommandBuffer = VK_NULL_HANDLE;
}
