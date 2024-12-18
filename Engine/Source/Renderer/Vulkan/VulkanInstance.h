#pragma once

class PVulkanInstance
{
public:
	PVulkanInstance()
	{		
		Instance = VK_NULL_HANDLE;
		Surface = VK_NULL_HANDLE;
		DebugMessenger = VK_NULL_HANDLE;
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