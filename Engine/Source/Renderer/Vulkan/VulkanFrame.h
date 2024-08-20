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
	void CreateFrame(PVulkanRHI* RHI);
	void DestroyFrame(PVulkanRHI* RHI);
	
	void BeginFrame(PVulkanRHI* RHI);
	void EndFrame(PVulkanRHI* RHI);

	PVulkanCommandBuffer* GetCommandBuffer() const;
	
	PVulkanCommandPool* CommandPool;
	PVulkanCommandBuffer* CommandBuffer;
	VkSemaphore SwapchainSemaphore;
	VkSemaphore RenderSemaphore;
	VkFence RenderFence;

	//std::optional<FTransientFrameData> TransientFrameData;
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

	void CreateFramePool(PVulkanRHI* InRHI);
	void FreeFramePool(PVulkanRHI* InRHI);

	std::vector<PVulkanFrame*> Pool;
	size_t FrameIndex;
	size_t PoolSize;
};