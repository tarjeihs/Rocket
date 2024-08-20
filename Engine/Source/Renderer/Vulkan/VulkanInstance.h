#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

class PVulkanRHI;

class PVulkanInstance
{
public:
	PVulkanInstance(PVulkanRHI* InRHI)
		: RHI(InRHI)
	{
		Instance = nullptr;
		Surface = nullptr;
		DebugMessenger = nullptr;
	}

	void Init();
	void Shutdown();

	VkInstance GetVkInstance() const;
	VkSurfaceKHR GetVkSurfaceKHR() const;

private:
	PVulkanRHI* RHI;

	VkInstance Instance;
	VkSurfaceKHR Surface;

	VkDebugUtilsMessengerEXT DebugMessenger;
};