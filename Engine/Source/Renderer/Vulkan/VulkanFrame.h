#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#include <optional>

class PVulkanCommandPool;
class PVulkanRHI;
class PVulkanCommandBuffer;

struct FTransientFrameData
{
	uint32_t NextImageIndex;
};

class PVulkanFrame
{
public:
	void CreateFrame();
	void DestroyFrame();
	
	void BeginFrame();
	void EndFrame();

	PVulkanCommandBuffer* GetCommandBuffer() const;
	
	PVulkanCommandPool* CommandPool;
	PVulkanCommandBuffer* CommandBuffer;
	VkSemaphore SwapchainSemaphore;
	VkSemaphore RenderSemaphore;
	VkFence RenderFence;

	FTransientFrameData TransientFrameData;
};

class PVulkanFramePool
{
public:
	PVulkanFramePool(size_t InPoolSize)
		: PoolSize(InPoolSize)
	{
		FrameIndex = 0;
	}

	void CreateFramePool();
	void FreeFramePool();

	std::vector<PVulkanFrame*> Pool;
	size_t FrameIndex;
	size_t PoolSize;
};