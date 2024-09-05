#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

class PVulkanRHI;

class PVulkanImage
{
public:
	void Init(VkExtent2D Extent, VkFormat Format);
	void Reset();

	void ApplyImage(VkImage Image);
	void ApplyImageView(VkImageView ImageView);

	void CreateImage(VkImageUsageFlags ImageUsageFlags);
	void CreateImageView(VkImageAspectFlags ImageViewAspectFlags);
	
	void DestroyImage();
	void DestroyImageView();

	void TransitionImageLayout(VkCommandBuffer CommandBuffer, VkImageLayout CurrentLayout, VkImageLayout NewLayout);
	void CopyImageRegion(VkCommandBuffer CommandBuffer, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize);

	VkImage GetVkImage() const;
	VkImageView GetVkImageView() const;
	VkExtent2D GetImageExtent2D() const;
	VkFormat GetVkFormat() const;

private:
	VkImage ImageHandle;
	VkImageView ImageViewHandle;
	VmaAllocation MemoryAllocation;
	VkFormat ImageFormat;
	VkExtent2D ImageExtent;
	VkExtent3D ImageExtent3D;
};