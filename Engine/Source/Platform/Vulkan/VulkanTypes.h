#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

struct SVulkanImage
{
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
	VkExtent3D imageExtent;
	VkFormat imageFormat;
};