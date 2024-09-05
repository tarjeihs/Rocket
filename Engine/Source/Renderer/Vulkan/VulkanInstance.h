#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

class PVulkanInstance
{
public:
	PVulkanInstance()
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
	VkInstance Instance;
	VkSurfaceKHR Surface;
	VkDebugUtilsMessengerEXT DebugMessenger;
};