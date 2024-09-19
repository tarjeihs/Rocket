#pragma once

struct VkCommandBuffer_T;
struct VkCommandPool_T;

typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkCommandPool_T* VkCommandPool;
typedef uint32_t VkCommandPoolCreateFlags;

class PVulkanCommandPool
{
public:
	void Create(uint32_t QueueFamilyIndex, VkCommandPoolCreateFlags Flags = 0);
	void Destroy();

	VkCommandPool GetVkCommandPool() const;

private:
	VkCommandPool CommandPool;
};

class PVulkanCommandBuffer
{
public:
	void Create(PVulkanCommandPool* CommandPool);
	void Destroy(PVulkanCommandPool* CommandPool);

	void ResetCommandBuffer();
	void BeginCommandBuffer();
	void EndCommandBuffer();
	
	VkCommandBuffer GetVkCommandBuffer() const;

private:
	VkCommandBuffer CommandBuffer;
};