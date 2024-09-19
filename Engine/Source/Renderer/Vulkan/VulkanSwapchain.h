#pragma once

#include <vector>

class PVulkanRHI;
class PVulkanImage;

struct VkSwapchainKHR_T;
struct VkSurfaceFormatKHR;
struct VkExtent2D;
enum VkPresentModeKHR;

typedef struct VkSwapchainKHR_T* VkSwapchainKHR;

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
	VkSwapchainKHR SwapchainKHR;
	
	// Defines pixel format of the images in the swapchain. (Color format and Depth/Stencil format)
	VkSurfaceFormatKHR SwapchainSurfaceFormat;
	
	// Width and height (in pixels) of the images in the swapchain
	VkExtent2D SwapchainImageExtent;
	
	VkPresentModeKHR SwapchainPresentMode;

	std::vector<PVulkanImage*> SwapchainImages;
};