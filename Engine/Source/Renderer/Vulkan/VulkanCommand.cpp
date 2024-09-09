#include "EnginePCH.h"
#include "VulkanCommand.h"

#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanFrame.h"

void PVulkanCommandPool::Create(uint32_t QueueFamilyIndex, VkCommandPoolCreateFlags Flags)
{
	// Create a command pool for command buffers to be submitted to the graphics queue.
	VkCommandPoolCreateInfo CommandPoolCreateInfo = {};
	CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolCreateInfo.pNext = nullptr;
	CommandPoolCreateInfo.flags = Flags;
	CommandPoolCreateInfo.queueFamilyIndex = QueueFamilyIndex;

	VkResult Result = vkCreateCommandPool(GetRHI()->GetDevice()->GetVkDevice(), &CommandPoolCreateInfo, nullptr, &CommandPool);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create command pool.");
}

void PVulkanCommandPool::Destroy()
{
	vkDestroyCommandPool(GetRHI()->GetDevice()->GetVkDevice(), CommandPool, nullptr);
	CommandPool = VK_NULL_HANDLE;
}

VkCommandPool PVulkanCommandPool::GetVkCommandPool() const
{
	return CommandPool;
}

void PVulkanCommandBuffer::Create(PVulkanCommandPool* CommandPool)
{
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandPool = CommandPool->GetVkCommandPool();
	CommandBufferAllocateInfo.commandBufferCount = 1;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkResult Result = vkAllocateCommandBuffers(GetRHI()->GetDevice()->GetVkDevice(), &CommandBufferAllocateInfo, &CommandBuffer);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate command buffers.");
}

void PVulkanCommandBuffer::Destroy(PVulkanCommandPool* CommandPool)
{
	vkFreeCommandBuffers(GetRHI()->GetDevice()->GetVkDevice(), CommandPool->GetVkCommandPool(), 1, &CommandBuffer);
	CommandBuffer = VK_NULL_HANDLE;
}

VkCommandBuffer PVulkanCommandBuffer::GetVkCommandBuffer() const
{
	return CommandBuffer;
}

void PVulkanCommandBuffer::ResetCommandBuffer()
{
	VkResult Result = vkResetCommandBuffer(CommandBuffer, 0);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to reset command buffer.");
}

void PVulkanCommandBuffer::BeginCommandBuffer()
{
	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	CommandBufferBeginInfo.pNext = nullptr;
	CommandBufferBeginInfo.pInheritanceInfo = nullptr;

	VkResult Result = vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to begin recording command buffer.");
}

void PVulkanCommandBuffer::EndCommandBuffer()
{
	VkResult Result = vkEndCommandBuffer(CommandBuffer);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to finalize command buffer.");
}
