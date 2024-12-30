#pragma once

class PVulkanRHI;
struct FVkImage;

struct FVkSwapchainInfo
{
	VkSwapchainKHR SwapchainKHR;
	VkSurfaceFormatKHR SwapchainSurfaceFormat;
	VkExtent2D SwapchainImageExtent;
	VkPresentModeKHR SwapchainPresentMode;
	TArray<FVkImage*> Backbuffer;
};

class PVulkanSwapchain
{
public:
	FVkSwapchainInfo Info;

	void Init();
	void Shutdown();
};