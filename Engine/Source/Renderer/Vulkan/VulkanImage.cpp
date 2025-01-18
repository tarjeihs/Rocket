#include "EnginePCH.h"
#include "VulkanImage.h"

#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Renderer/Vulkan/VulkanCommand.h"

namespace Utils
{
	VkFormat GetVkFormat(EImageFormat Format) 
	{
		switch (Format) 
		{
			case EImageFormat::R16G16B16A16_SFLOAT:					return VK_FORMAT_R16G16B16A16_SFLOAT;
			case EImageFormat::R16G16B16A16_UNORM:					return VK_FORMAT_R16G16B16A16_UNORM;
			case EImageFormat::R16G16B16_SFLOAT:					return VK_FORMAT_R16G16B16_SFLOAT;
			case EImageFormat::R16G16B16_UNORM:						return VK_FORMAT_R16G16B16_UNORM;
			case EImageFormat::R16G16_SFLOAT:						return VK_FORMAT_R16G16_SFLOAT;
			case EImageFormat::R16G16_UNORM:						return VK_FORMAT_R16G16_UNORM;
			case EImageFormat::R16_SFLOAT:							return VK_FORMAT_R16_SFLOAT;
			case EImageFormat::R16_UNORM:							return VK_FORMAT_R16_UNORM;
			case EImageFormat::R8G8B8A8_SRGB:						return VK_FORMAT_R8G8B8A8_SRGB;
			case EImageFormat::R8G8B8A8_UNORM:						return VK_FORMAT_R8G8B8A8_UNORM;
			case EImageFormat::R8G8B8_SRGB:							return VK_FORMAT_R8G8B8_SRGB;
			case EImageFormat::R8G8B8_UNORM:						return VK_FORMAT_R8G8B8_UNORM;
			case EImageFormat::R8G8_SRGB:							return VK_FORMAT_R8G8_SRGB;
			case EImageFormat::R8G8_UNORM:							return VK_FORMAT_R8G8_UNORM;
			case EImageFormat::R8_SRGB:								return VK_FORMAT_R8_SRGB;
			case EImageFormat::R8_UNORM:							return VK_FORMAT_R8_UNORM;
			case EImageFormat::D32_SFLOAT:									return VK_FORMAT_D32_SFLOAT;
			case EImageFormat::A2B10G10R10_UNORM_PACK32:			return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			case EImageFormat::A2R10G10B10_UNORM_PACK32:			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		}
		
		return VK_FORMAT_UNDEFINED;
	}

	VkImageLayout GetVkImageLayout(EImageLayout ImageLayout)
	{
		switch (ImageLayout)
		{
			case EImageLayout::Undefined:	return VK_IMAGE_LAYOUT_UNDEFINED;
			case EImageLayout::General:		return VK_IMAGE_LAYOUT_GENERAL;
			case EImageLayout::ReadOnly:	return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		}

		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	VkImageUsageFlags GetVkImageUsageFlags(EImageUsage ImageUsageFlags)
	{
		VkImageUsageFlags Flags = 0;

		if (HasFlag(ImageUsageFlags, EImageUsage::TransferSrc))		Flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (HasFlag(ImageUsageFlags, EImageUsage::TransferDst))		Flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (HasFlag(ImageUsageFlags, EImageUsage::Storage))			Flags |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (HasFlag(ImageUsageFlags, EImageUsage::Sampled))			Flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (HasFlag(ImageUsageFlags, EImageUsage::ColorAttachment))			Flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (HasFlag(ImageUsageFlags, EImageUsage::DepthStencilAttachment))			Flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		return Flags;
	}

	VkImageAspectFlags GetVkImageAspectFlags(EImageAspect ImageAspectFlags)
	{
		VkImageAspectFlags Flags = 0;
		
		if (HasFlag(ImageAspectFlags, EImageAspect::Color))				Flags |= VK_IMAGE_ASPECT_COLOR_BIT;
		if (HasFlag(ImageAspectFlags, EImageAspect::Depth))				Flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
		if (HasFlag(ImageAspectFlags, EImageAspect::Stencil))			Flags |= VK_IMAGE_ASPECT_STENCIL_BIT;

		return Flags;
	}
}

void FVkImage::Initialize(FImageCreateInfo& CreateInfo)
{
	VkImageCreateInfo ImageCreateInfo = {};
	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.format = Utils::GetVkFormat(CreateInfo.Format);
	ImageCreateInfo.extent = VkExtent3D(CreateInfo.Extent.Width, CreateInfo.Extent.Height, 1);
	ImageCreateInfo.mipLevels = 1;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageCreateInfo.usage = Utils::GetVkImageUsageFlags(CreateInfo.UsageFlags);

	VmaAllocationCreateInfo ImageAllocationCreateInfo = {};
	ImageAllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	ImageAllocationCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkResult Result = vmaCreateImage(GetRHI()->GetAllocator()->GetMemoryAllocator(), &ImageCreateInfo, &ImageAllocationCreateInfo, &Info.ImageHandle, &Info.MemoryAllocation, nullptr);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create image.");

	VkImageViewCreateInfo ImageViewCreateInfo = {};
	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ImageViewCreateInfo.image = Info.ImageHandle;
	ImageViewCreateInfo.format = Utils::GetVkFormat(CreateInfo.Format);
	ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 1;
	ImageViewCreateInfo.subresourceRange.aspectMask = Utils::GetVkImageAspectFlags(CreateInfo.AspectFlags);

	Result = vkCreateImageView(GetRHI()->GetDevice()->GetVkDevice(), &ImageViewCreateInfo, nullptr, &Info.ImageViewHandle);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");

	Info.Extent = VkExtent2D(CreateInfo.Extent.Width, CreateInfo.Extent.Height);
	Info.Format = Utils::GetVkFormat(CreateInfo.Format);
}

void FVkImage::Shutdown()
{
	vmaDestroyImage(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.ImageHandle, Info.MemoryAllocation);
	vkDestroyImageView(GetRHI()->GetDevice()->GetVkDevice(), Info.ImageViewHandle, VK_NULL_HANDLE);
}

void FVkImage::TransitionImageLayout(FVkCommandBuffer* CommandBuffer, VkImageLayout CurrentLayout, VkImageLayout NewLayout, VkAccessFlags2 SrcAccessMask, VkAccessFlags2 DstAccessMask, VkPipelineStageFlags2 SrcStageMask, VkPipelineStageFlags2 DstStageMask)
{
	VkImageSubresourceRange SubresourceRange = {};
	SubresourceRange.aspectMask = (NewLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	SubresourceRange.baseMipLevel = 0;
	SubresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	SubresourceRange.baseArrayLayer = 0;
	SubresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	VkImageMemoryBarrier2 ImageMemoryBarrier = {};
	ImageMemoryBarrier.pNext = nullptr;
	ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	ImageMemoryBarrier.srcAccessMask = SrcAccessMask;
	ImageMemoryBarrier.dstAccessMask = DstAccessMask;
	ImageMemoryBarrier.srcStageMask = SrcStageMask;
	ImageMemoryBarrier.dstStageMask = DstStageMask;
	ImageMemoryBarrier.oldLayout = CurrentLayout;
	ImageMemoryBarrier.newLayout = NewLayout;
	ImageMemoryBarrier.subresourceRange = SubresourceRange;
	ImageMemoryBarrier.image = Info.ImageHandle;

	VkDependencyInfo DependencyInfo = {};
	DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	DependencyInfo.pNext = nullptr;
	DependencyInfo.imageMemoryBarrierCount = 1;
	DependencyInfo.pImageMemoryBarriers = &ImageMemoryBarrier;

	vkCmdPipelineBarrier2(CommandBuffer->GetVkCommandBuffer(), &DependencyInfo);
}

void FVkImage::CopyImageRegion(FVkCommandBuffer* CommandBuffer, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize)
{
    VkImageBlit2 ImageBlit = {};
    ImageBlit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
    ImageBlit.pNext = nullptr;

    // Source
    ImageBlit.srcOffsets[0] = {0, 0, 0};
    ImageBlit.srcOffsets[1] = {static_cast<int32_t>(SrcSize.width), static_cast<int32_t>(SrcSize.height), 1};
    ImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageBlit.srcSubresource.mipLevel = 0;
    ImageBlit.srcSubresource.baseArrayLayer = 0;
    ImageBlit.srcSubresource.layerCount = 1;

    // Destination
    ImageBlit.dstOffsets[0] = {0, 0, 0};
    ImageBlit.dstOffsets[1] = {static_cast<int32_t>(DstSize.width), static_cast<int32_t>(DstSize.height), 1};
    ImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageBlit.dstSubresource.mipLevel = 0;
    ImageBlit.dstSubresource.baseArrayLayer = 0;
    ImageBlit.dstSubresource.layerCount = 1;

    VkBlitImageInfo2 BlitImageInfo = {};
    BlitImageInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
    BlitImageInfo.pNext = nullptr;
    BlitImageInfo.srcImage = Info.ImageHandle;
    BlitImageInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    BlitImageInfo.dstImage = Dest;
    BlitImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    BlitImageInfo.filter = VK_FILTER_LINEAR;
    BlitImageInfo.regionCount = 1;
    BlitImageInfo.pRegions = &ImageBlit;

    vkCmdBlitImage2(CommandBuffer->GetVkCommandBuffer(), &BlitImageInfo);
}