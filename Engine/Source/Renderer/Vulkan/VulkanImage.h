#pragma once

class PVulkanRHI;
class PVulkanCommandBuffer;

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

struct FVkImage
{
	FVkImageInfo Info;

	void Initialize(FVkImageCreateInfo& CreateInfo);
	void Shutdown();

	void TransitionImageLayout(PVulkanCommandBuffer* CommandBuffer, VkImageLayout CurrentLayout, VkImageLayout NewLayout, VkAccessFlags2 SrcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT, VkAccessFlags2 DstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT, VkPipelineStageFlags2 SrcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VkPipelineStageFlags2 DstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
	void CopyImageRegion(PVulkanCommandBuffer* CommandBuffer, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize);
};