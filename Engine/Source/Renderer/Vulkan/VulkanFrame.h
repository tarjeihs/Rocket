#pragma once

#include <vector>

// Forward declaration
struct SVulkanBuffer;
class PVulkanDescriptorPool;
class PVulkanDescriptorSetLayout;
class PVulkanDescriptorSet;
class PVulkanCommandPool;
class PVulkanRHI;
class PVulkanCommandBuffer;
struct VkSemaphore_T;
struct VkFence_T;
typedef struct VkSemaphore_T* VkSemaphore;
typedef struct VkFence_T* VkFence;

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

	PVulkanDescriptorSet* DescriptorSet;
	SVulkanBuffer* SSBO;
	SVulkanBuffer* UBO;

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


	PVulkanDescriptorSetLayout* DescriptorSetLayout;

	std::vector<PVulkanFrame*> Pool;
	size_t FrameIndex;
	size_t PoolSize;
};