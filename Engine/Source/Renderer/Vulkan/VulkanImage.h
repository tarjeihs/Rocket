#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

class PVulkanRHI;

class PVulkanImage
{
public:
	void CreateImage(PVulkanRHI* RHI, VkExtent2D Extent);
	void DestroyImage(PVulkanRHI* RHI);

	void TransitionImageLayout(VkCommandBuffer CommandBuffer, VkImageLayout CurrentLayout, VkImageLayout NewLayout);
	void CopyImageRegion(VkCommandBuffer CommandBuffer, VkImage Src, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize);

	VkImage Image;
	VkImageView ImageView;
	VkExtent2D ImageExtent;
	VkFormat ImageFormat;
	VmaAllocation MemoryAllocation;
};