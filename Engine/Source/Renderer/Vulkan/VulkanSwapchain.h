#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

class PVulkanRHI;
class PVulkanImage;

class PVulkanSwapchain
{
public:
	void Init();
	void Shutdown();

	VkSwapchainKHR GetVkSwapchain() const;
	VkExtent2D GetVkExtent() const;
	VkSurfaceFormatKHR GetSurfaceFormat() const;

	const std::vector<PVulkanImage*>& GetSwapchainImages() const;

private:
	VkSwapchainKHR SwapchainKHR;																				// Image 1: (On Display): Image is currently being shown on the screen. Image 2: (In the pipeline): GPU is currently rendering onto this image. Image 3: (Waiting) Image is ready and waiting.
	VkSurfaceFormatKHR SwapchainSurfaceFormat;																			// Defines pixel format of the images in the swapchain. (Color format and Depth/Stencil format)
	VkPresentModeKHR SwapchainPresentMode;
	VkExtent2D SwapchainImageExtent;																			// Defines the width and height (in pixels) of the images in the swapchain.

	std::vector<PVulkanImage*> SwapchainImages;
};