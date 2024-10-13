#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

// Forward declaration
class PVulkanCommandPool;
class PVulkanRHI;
class PVulkanCommandBuffer;
class PVulkanMemory;

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

	PVulkanCommandPool* GetCommandPool() const;
	PVulkanCommandBuffer* GetCommandBuffer() const;
	PVulkanMemory* GetMemory() const;

	VkSemaphore GetSwapchainSemaphore() const;
	VkSemaphore GetRenderSemaphore() const;
	VkFence GetRenderFence() const;

	FTransientFrameData& GetTransientFrameData();

private:
	PVulkanCommandPool* CommandPool;
	PVulkanCommandBuffer* CommandBuffer;
	VkSemaphore SwapchainSemaphore;
	VkSemaphore RenderSemaphore;
	VkFence RenderFence;
	PVulkanMemory* Memory;
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
	
	PVulkanFrame* GetCurrentFrame() const;
	size_t GetCurrentFrameIndex() const;

    std::vector<PVulkanFrame*>::iterator begin() { return Pool.begin(); }
    std::vector<PVulkanFrame*>::const_iterator begin() const { return Pool.begin(); }

    std::vector<PVulkanFrame*>::iterator end() { return Pool.end(); }
    std::vector<PVulkanFrame*>::const_iterator end() const { return Pool.end(); }

private:
	std::vector<PVulkanFrame*> Pool;
	size_t FrameIndex;
	size_t PoolSize;

	friend class PVulkanSceneRenderer;
};