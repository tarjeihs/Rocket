#include "VulkanImage.h"

#include "Core/Assert.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanImage::CreateImage(PVulkanRHI* RHI, VkExtent2D Extent)
{
	VkExtent3D ImageExtent3D = { Extent.width, Extent.height, 1 };

	ImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	ImageExtent = Extent;

	VkImageUsageFlags ImageUsageFlags{};
	ImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	ImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	ImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	ImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo ImageCreateInfo{};
	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.format = ImageFormat;
	ImageCreateInfo.extent = ImageExtent3D;
	ImageCreateInfo.mipLevels = 1;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageCreateInfo.usage = ImageUsageFlags;

	// Allocate and create the image
	VmaAllocationCreateInfo ImageAllocationCreateInfo{};
	ImageAllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	ImageAllocationCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vmaCreateImage(RHI->GetMemory()->GetMemoryAllocator(), &ImageCreateInfo, &ImageAllocationCreateInfo, &Image, &MemoryAllocation, nullptr);

	// Create image view
	VkImageViewCreateInfo ImageViewCreateInfo{};
	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ImageViewCreateInfo.image = Image;
	ImageViewCreateInfo.format = ImageFormat;
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 1;
	ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkResult Result = vkCreateImageView(RHI->GetDevice()->GetVkDevice(), &ImageViewCreateInfo, nullptr, &ImageView);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");
}

void PVulkanImage::DestroyImage(PVulkanRHI* RHI)
{
	vkDestroyImageView(RHI->GetDevice()->GetVkDevice(), ImageView, nullptr);
	vmaDestroyImage(RHI->GetMemory()->GetMemoryAllocator(), Image, MemoryAllocation);
}

void PVulkanImage::TransitionImageLayout(VkCommandBuffer CommandBuffer, VkImageLayout CurrentLayout, VkImageLayout NewLayout)
{
	VkImageSubresourceRange SubresourceRange{};
	SubresourceRange.aspectMask = (NewLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	SubresourceRange.baseMipLevel = 0;
	SubresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	SubresourceRange.baseArrayLayer = 0;
	SubresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	VkImageMemoryBarrier2 ImageMemoryBarrier{};
	ImageMemoryBarrier.pNext = nullptr;
	ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	ImageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	ImageMemoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	ImageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	ImageMemoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	ImageMemoryBarrier.oldLayout = CurrentLayout;
	ImageMemoryBarrier.newLayout = NewLayout;
	ImageMemoryBarrier.subresourceRange = SubresourceRange;
	ImageMemoryBarrier.image = Image;

	VkDependencyInfo DependencyInfo{};
	DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	DependencyInfo.pNext = nullptr;
	DependencyInfo.imageMemoryBarrierCount = 1;
	DependencyInfo.pImageMemoryBarriers = &ImageMemoryBarrier;

	vkCmdPipelineBarrier2(CommandBuffer, &DependencyInfo);
}

void PVulkanImage::CopyImageRegion(VkCommandBuffer CommandBuffer, VkImage Src, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize)
{
	VkImageBlit2 ImageBlit{};
	ImageBlit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
	ImageBlit.pNext = nullptr;
	ImageBlit.srcOffsets[1].x = SrcSize.width;
	ImageBlit.srcOffsets[1].y = SrcSize.height;
	ImageBlit.srcOffsets[1].z = 1;
	ImageBlit.dstOffsets[1].x = DstSize.width;
	ImageBlit.dstOffsets[1].y = DstSize.height;
	ImageBlit.dstOffsets[1].z = 1;
	ImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ImageBlit.srcSubresource.baseArrayLayer = 0;
	ImageBlit.srcSubresource.layerCount = 1;
	ImageBlit.srcSubresource.mipLevel = 0;
	ImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ImageBlit.dstSubresource.baseArrayLayer = 0;
	ImageBlit.dstSubresource.layerCount = 1;
	ImageBlit.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 ImageBlitInfo{};
	ImageBlitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	ImageBlitInfo.pNext = nullptr;
	ImageBlitInfo.dstImage = Dest;
	ImageBlitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	ImageBlitInfo.srcImage = Src;
	ImageBlitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	ImageBlitInfo.filter = VK_FILTER_LINEAR;
	ImageBlitInfo.regionCount = 1;
	ImageBlitInfo.pRegions = &ImageBlit;

	vkCmdBlitImage2(CommandBuffer, &ImageBlitInfo);
}
