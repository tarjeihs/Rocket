#pragma once

#include <vulkan/vulkan_core.h>

class PVulkanRHI;
class PVulkanFrame;

class PVulkanCommandPool
{
public:
	void Create(VkDevice Device, uint32_t QueueFamilyIndex, VkCommandPoolCreateFlags Flags = 0);
	void Destroy(VkDevice Device);

	VkCommandPool GetVkCommandPool() const;

private:
	VkCommandPool CommandPool;
};

class PVulkanCommandBuffer
{
public:
	void Create(VkDevice Device, VkCommandPool CommandPool);
	void Destroy(VkDevice Device, VkCommandPool CommandPool);

	VkCommandBuffer GetVkCommandBuffer() const;

	void BeginCommandBuffer();
	void EndCommandBuffer();

private:
	VkCommandBuffer CommandBuffer;
};

struct SVulkanCommandQueue
{
	
};