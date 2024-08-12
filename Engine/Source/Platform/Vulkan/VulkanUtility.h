#pragma once

#include <optional>
#include <vulkan/vulkan_core.h>
#include <vector>

struct SQueueFamilyIndices
{
	std::optional<uint32_t> GraphicsFamily;
	std::optional<uint32_t> PresentFamily;

	inline bool IsComplete() const
	{
		return GraphicsFamily.has_value() && PresentFamily.has_value();
	}
};

struct SSwapchainSupportDetails
{
	// Swapchain capabilities (eg. min/max images in swap chain, min/max resolution of images)
	VkSurfaceCapabilitiesKHR Capabilities;

	// Pixel format, color depth
	std::vector<VkSurfaceFormatKHR> Formats;

	// Conditions for "swapping" images to the screen
	std::vector<VkPresentModeKHR> PresentMode;
};

namespace Vulkan
{
	namespace Utility
	{
		SQueueFamilyIndices RequestQueueFamilies(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice);
		SSwapchainSupportDetails RequestSwapchainSupportDetails(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice);
		bool IsVulkanCapableDevice(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice, const std::vector<const char*>& Extensions);

		void TransitionImage(VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout CurrentLayout, VkImageLayout NewLayout);
		void CopyImageToImage(VkCommandBuffer CommandBuffer, VkImage Src, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize);
		VkImageSubresourceRange CreateSubresourceRange(VkImageAspectFlags AspectMasks);
	}
}