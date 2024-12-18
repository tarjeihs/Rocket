#pragma once

#include <vector>

class PVulkanRHI;
class FVkImage;

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

	const std::vector<FVkImage*>& GetSwapchainImages() const;

private:
	VkSwapchainKHR SwapchainKHR;
	VkSurfaceFormatKHR SwapchainSurfaceFormat;
	VkExtent2D SwapchainImageExtent;
	VkPresentModeKHR SwapchainPresentMode;
	std::vector<FVkImage*> SwapchainImages; // TODO: Backbuffer
};