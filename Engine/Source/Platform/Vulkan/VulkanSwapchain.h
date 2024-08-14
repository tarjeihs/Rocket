#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "VulkanTypes.h"

class PVulkanSwapchain
{
public:
	void CreateSwapchain(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice, VmaAllocator Allocator);
	void RegenerateSwapchain(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice, VmaAllocator Allocator);
	void DestroySwapchain(VkSurfaceKHR SurfaceInterface, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice, VmaAllocator Allocator);

protected:
	VkSurfaceFormatKHR SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats);
	VkPresentModeKHR SelectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& PresentModes);
	VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

public:
	VkSwapchainKHR SwapchainKHR;                                                    // Image 1: (On Display): Image is currently being shown on the screen. Image 2: (In the pipeline): GPU is currently rendering onto this image. Image 3: (Waiting) Image is ready and waiting.
	VkFormat SwapchainImageFormat;                                                  // Defines pixel format of the images in the swapchain. (Color format and Depth/Stencil format)
	VkExtent2D SwapchainExtent;                                                     // Defines the width and height (in pixels) of the images in the swapchain.

	std::vector<VkImage> SwapchainImages;
	std::vector<VkImageView> SwapchainImageViews;

	// TODO: Should this RenderTarget not be cleaned up?
	SVulkanImage DrawImage; // render target
	VkExtent2D DrawImageExtent;                                                     // Defines the width and height (in pixels) of the images in the render target.
};