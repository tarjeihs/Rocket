#include "VulkanCommand.h"

#include "Core/Assert.h"
#include "Renderer/Vulkan/VulkanFrame.h"

void PVulkanCommandPool::Create(VkDevice Device, uint32_t QueueFamilyIndex, VkCommandPoolCreateFlags Flags)
{
	// Create a command pool for command buffers to be submitted to the graphics queue.
	VkCommandPoolCreateInfo CommandPoolCreateInfo = {};
	CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolCreateInfo.pNext = nullptr;
	CommandPoolCreateInfo.flags = Flags;
	CommandPoolCreateInfo.queueFamilyIndex = QueueFamilyIndex;

	VkResult Result = vkCreateCommandPool(Device, &CommandPoolCreateInfo, nullptr, &CommandPool);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create command pool.");
}

void PVulkanCommandPool::Destroy(VkDevice Device)
{
	vkDestroyCommandPool(Device, CommandPool, nullptr);
	CommandPool = VK_NULL_HANDLE;
}

VkCommandPool PVulkanCommandPool::GetVkCommandPool() const
{
	return CommandPool;
}

void PVulkanCommandBuffer::Create(VkDevice Device, VkCommandPool CommandPool)
{
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandPool = CommandPool;
	CommandBufferAllocateInfo.commandBufferCount = 1;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkResult Result = vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CommandBuffer);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate command buffers.");
}

void PVulkanCommandBuffer::Destroy(VkDevice Device, VkCommandPool CommandPool)
{
	vkFreeCommandBuffers(Device, CommandPool, 1, &CommandBuffer);
	CommandBuffer = VK_NULL_HANDLE;
}

VkCommandBuffer PVulkanCommandBuffer::GetVkCommandBuffer() const
{
	return CommandBuffer;
}

void PVulkanCommandBuffer::BeginCommandBuffer()
{
	VkResult Result = vkResetCommandBuffer(CommandBuffer, 0);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to reset command buffer.");

	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	CommandBufferBeginInfo.pNext = nullptr;
	CommandBufferBeginInfo.pInheritanceInfo = nullptr;

	Result = vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to begin recording command buffer.");
}

void PVulkanCommandBuffer::EndCommandBuffer()
{
	VkResult Result = vkEndCommandBuffer(CommandBuffer);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to finalize command buffer.");
}
