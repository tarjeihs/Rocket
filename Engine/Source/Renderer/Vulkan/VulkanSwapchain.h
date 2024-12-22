#pragma once

#include <vector>

class PVulkanRHI;
class PVulkanImage;

struct FVkSwapchainInfo
{

};

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
	VkSurfaceFormatKHR SwapchainSurfaceFormat;
	VkExtent2D SwapchainImageExtent;
	VkPresentModeKHR SwapchainPresentMode;
	std::vector<PVulkanImage*> SwapchainImages; // TODO: Backbuffer
};