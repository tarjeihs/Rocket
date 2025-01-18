#pragma once

class FVkCommandPool
{
public:
	void Initialize(uint32_t QueueFamilyIndex, VkCommandPoolCreateFlags Flags = 0);
	void Shutdown();

	VkCommandPool GetVkCommandPool() const;

private:
	VkCommandPool CommandPool;
};

class FVkCommandBuffer
{
public:
	void Initialize(FVkCommandPool* CommandPool);
	void Shutdown(FVkCommandPool* CommandPool);

	void ResetCommandBuffer();
	void BeginCommandBuffer();
	void EndCommandBuffer();
	
	VkCommandBuffer GetVkCommandBuffer() const;

private:
	VkCommandBuffer CommandBuffer;
};