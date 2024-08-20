#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

class PVulkanRHI;

class PVulkanSwapchain
{
public:
	void Init(PVulkanRHI* RHI);
	void Shutdown(PVulkanRHI* RHI);

	VkSwapchainKHR GetVkSwapchain() const;
	VkSurfaceFormatKHR GetSurfaceFormat() const;
	VkExtent2D GetVkExtent() const;

	const std::vector<VkImage>& GetSwapchainImages() const;
	const std::vector<VkImageView>& GetSwapchainImageViews() const;

private:
	VkSwapchainKHR SwapchainKHR;																	// Image 1: (On Display): Image is currently being shown on the screen. Image 2: (In the pipeline): GPU is currently rendering onto this image. Image 3: (Waiting) Image is ready and waiting.
	VkSurfaceFormatKHR SurfaceFormat;																// Defines pixel format of the images in the swapchain. (Color format and Depth/Stencil format)
	VkPresentModeKHR PresentMode;
	VkExtent2D ImageExtent;																			// Defines the width and height (in pixels) of the images in the swapchain.

	std::vector<VkImage> SwapchainImages;
	std::vector<VkImageView> SwapchainImageViews;
};