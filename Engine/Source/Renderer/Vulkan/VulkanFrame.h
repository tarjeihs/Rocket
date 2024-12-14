#pragma once

#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
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

static const uint32 BINDLESS_DESCRIPTOR_SET_INDEX_STORAGE_BUFFER 	= 0;
static const uint32 BINDLESS_DESCRIPTOR_SET_INDEX_STORAGE_IMAGE 	= 1;
static const uint32 BINDLESS_DESCRIPTOR_SET_INDEX_SAMPLER 			= 2;
static const uint32 BINDLESS_DESCRIPTOR_SET_INDEX_SAMPLER_IMAGE 	= 3;

static const uint32 STORAGE_BUFFER_DESCRIPTOR_INDEX_GLOBAL 			= 0;
static const uint32 STORAGE_BUFFER_DESCRIPTOR_INDEX_CAMERA 			= 1;
static const uint32 STORAGE_BUFFER_DESCRIPTOR_INDEX_MATERIAL 		= 2;
static const uint32 STORAGE_BUFFER_DESCRIPTOR_INDEX_OBJECT 			= 3;

class PVulkanFrame
{
public:
	TSharedPtr<FVkDescriptorSet> UniformBufferDescriptorSet;
	TSharedPtr<FVkDescriptorSet> StorageBufferDescriptorSet;

	TSharedPtr<FVkBuffer> GlobalStorageBuffer;
	TSharedPtr<FVkBuffer> CameraStorageBuffer;

	TSharedPtr<FVkBuffer> ObjectStorageBuffer;
	TSharedPtr<FVkBuffer> MaterialStorageBuffer;

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

	TSharedPtr<FVkPipelineLayout> GraphicsPipelineLayout;
	TSharedPtr<FVkPipeline> GraphicsPipeline;
private:

	std::vector<PVulkanFrame*> Pool;
	size_t FrameIndex;
	size_t PoolSize;

	friend class PVulkanSceneRenderer;
};