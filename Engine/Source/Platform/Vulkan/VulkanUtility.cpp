#include "EnginePCH.h"
#include "VulkanUtility.h"
#include <set>

SQueueFamilyIndices Vulkan::Utility::RequestQueueFamilies(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice)
{
	SQueueFamilyIndices Indices;

	uint32_t QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilies.data());

	for (int32_t Index = 0; Index < QueueFamilyCount; Index++)
	{
		const VkQueueFamilyProperties& FamilyProperty = QueueFamilies[Index];
		if (FamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			Indices.GraphicsFamily = Index;
		}

		VkBool32 PresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, Index, SurfaceInterface, &PresentSupport);

		if (PresentSupport)
		{
			Indices.PresentFamily = Index;
		}

		if (Indices.IsComplete())
		{
			break;
		}
	}
	return Indices;
}

SSwapchainSupportDetails Vulkan::Utility::RequestSwapchainSupportDetails(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice)
{
	SSwapchainSupportDetails SwapChainSupportDetails;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, SurfaceInterface, &SwapChainSupportDetails.Capabilities);

	uint32_t FormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, SurfaceInterface, &FormatCount, nullptr);

	if (FormatCount != 0)
	{
		SwapChainSupportDetails.Formats.resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, SurfaceInterface, &FormatCount, SwapChainSupportDetails.Formats.data());
	}

	uint32_t PresentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, SurfaceInterface, &PresentModeCount, SwapChainSupportDetails.PresentMode.data());
	if (PresentModeCount != 0)
	{
		SwapChainSupportDetails.PresentMode.resize(PresentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, SurfaceInterface, &PresentModeCount, SwapChainSupportDetails.PresentMode.data());
	}
	return SwapChainSupportDetails;
}

bool Vulkan::Utility::IsVulkanCapableDevice(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice, const std::vector<const char*>& Extensions)
{
	SQueueFamilyIndices Indices = RequestQueueFamilies(SurfaceInterface, PhysicalDevice);

	uint32_t ExtensionCount;
	vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, nullptr);

	std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
	vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

	std::set<std::string> RequiredExtensions(Extensions.begin(), Extensions.end());
	for (const auto& Extension : AvailableExtensions)
	{
		RequiredExtensions.erase(Extension.extensionName);
	}

	bool bExtensionSupport = RequiredExtensions.empty();
	bool bSwapChainAdequate = false;

	SSwapchainSupportDetails SwapchainSupport;
	if (bExtensionSupport)
	{
		SwapchainSupport = RequestSwapchainSupportDetails(SurfaceInterface, PhysicalDevice);
		bSwapChainAdequate = !SwapchainSupport.Formats.empty() && !SwapchainSupport.PresentMode.empty();
	}

	bool bIsCapable = Indices.IsComplete() && bExtensionSupport && bSwapChainAdequate;
	return bIsCapable;
}

void Vulkan::Utility::TransitionImage(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout CurrentLayout, VkImageLayout NewLayout)
{
	VkImageMemoryBarrier2 ImageBarrier{};
	ImageBarrier.pNext = nullptr;
	ImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

	ImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	ImageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	ImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	ImageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

	ImageBarrier.oldLayout = CurrentLayout;
	ImageBarrier.newLayout = NewLayout;

	ImageBarrier.subresourceRange = CreateSubresourceRange((NewLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT);
	ImageBarrier.image = Image;

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.pNext = nullptr;

	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &ImageBarrier;

	vkCmdPipelineBarrier2(CommandBuffer, &depInfo);
}

void Vulkan::Utility::CopyImageToImage(VkCommandBuffer CommandBuffer, VkImage Src, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize)
{
	VkImageBlit2 blitRegion{};
	blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
	blitRegion.pNext = nullptr;

	blitRegion.srcOffsets[1].x = SrcSize.width;
	blitRegion.srcOffsets[1].y = SrcSize.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = DstSize.width;
	blitRegion.dstOffsets[1].y = DstSize.height;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blitInfo{};
	blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	blitInfo.pNext = nullptr;
	blitInfo.dstImage = Dest;
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.srcImage = Src;
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

	vkCmdBlitImage2(CommandBuffer, &blitInfo);
}

VkImageSubresourceRange Vulkan::Utility::CreateSubresourceRange(VkImageAspectFlags AspectMasks)
{
	VkImageSubresourceRange SubImage{};
	SubImage.aspectMask = AspectMasks;
	SubImage.baseMipLevel = 0;
	SubImage.levelCount = VK_REMAINING_MIP_LEVELS;
	SubImage.baseArrayLayer = 0;
	SubImage.layerCount = VK_REMAINING_ARRAY_LAYERS;
	
	return SubImage;
}