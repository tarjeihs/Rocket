#pragma once

#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanPipeline.h"

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
	static const uint32 BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER_LOCATION = 1;
	static const uint32 BINDLESS_DESCRIPTOR_SET_SAMPLER_LOCATION = 2;
	static const uint32 BINDLESS_DESCRIPTOR_SET_SAMPLER_IMAGE_LOCATION = 3;
	static const uint32 BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE_LOCATION = 4;

	TSharedPtr<FVkDescriptorSet> BindlessDescriptorSetStorageBuffer;
	TSharedPtr<FVkDescriptorSet> BindlessDescriptorSetStorageImage;
	TSharedPtr<FVkDescriptorSet> BindlessDescriptorSetSampler;
	TSharedPtr<FVkDescriptorSet> BindlessDescriptorSetSamplerImage;

	void CreateFrame();
	void DestroyFrame();
	
	void BeginFrame();
	void EndFrame();

	PVulkanCommandPool* GetCommandPool() const;
	PVulkanCommandBuffer* GetCommandBuffer() const;

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
	PVulkanPipelineLayoutData* PipelineLayoutData;
	PVulkanPipelineStateData* PipelineStateData;

	std::vector<PVulkanFrame*> Pool;
	size_t FrameIndex;
	size_t PoolSize;

	friend class PVulkanSceneRenderer;
};