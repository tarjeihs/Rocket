#pragma once

struct VkInstance_T;
struct VkSurfaceKHR_T;
struct VkDebugUtilsMessengerEXT_T;

typedef struct VkInstance_T* VkInstance;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;

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