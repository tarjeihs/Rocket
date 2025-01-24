#pragma once

#include "Renderer/Common/Image.h"

class PVulkanRHI;
class FVkCommandBuffer;

struct FVkImageCreateInfo
{
	VkImageLayout ImageLayout;
	VkImageUsageFlags ImageUsageFlags;
	VkImageAspectFlags ImageViewAspectFlags;
	VkExtent2D Extent;
	VkFormat Format;
};

struct FVkImageInfo
{
	VkImage ImageHandle;
	VkImageView ImageViewHandle;
	VmaAllocation MemoryAllocation;
	VkSampler Sampler;
	VkExtent2D Extent;
	VkFormat Format;
};

struct FVkImage : public IImage
{
	FVkImageInfo Info;

	virtual void Initialize(const FImageCreateInfo& CreateInfo) override;
	virtual void Shutdown() override;

	void TransitionImageLayout(FVkCommandBuffer* CommandBuffer, VkImageLayout CurrentLayout, VkImageLayout NewLayout, VkAccessFlags2 SrcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT, VkAccessFlags2 DstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT, VkPipelineStageFlags2 SrcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VkPipelineStageFlags2 DstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
	void CopyImageRegion(FVkCommandBuffer* CommandBuffer, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize);
};