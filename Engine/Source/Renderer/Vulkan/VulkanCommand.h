#pragma once

#include <vulkan/vulkan_core.h>

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

	void ResetCommandBuffer();
	void BeginCommandBuffer();
	void EndCommandBuffer();
	
	VkCommandBuffer GetVkCommandBuffer() const;

private:
	VkCommandBuffer CommandBuffer;
};