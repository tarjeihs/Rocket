#include "EnginePCH.h"
#include "VulkanInstance.h"

#include "Core/Assert.h"
#include "Core/Window.h"
#include "Renderer/RHI.h"
#include "Renderer/VulkanRHI.h"

namespace Utils
{
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (Func != nullptr)
		{
			return Func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (Func != nullptr)
		{
			Func(instance, debugMessenger, pAllocator);
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
	{
		switch (Severity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	{ RK_LOG_VERBOSE(CallbackData->pMessage); break; }
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		{ RK_LOG_INFO(CallbackData->pMessage);   break; }
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	{ RK_LOG_WARNING(CallbackData->pMessage); break; }
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		{ RK_LOG_ERROR(CallbackData->pMessage);  break; }
		}
		return VK_FALSE;
	}
}

void PVulkanInstance::Init()
{
	// GLFW required Vulkan extensions
	uint32_t GlfwExtensionCount = 0;
	const char** GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);
	GetRHI()->Extensions.InstanceExtensions.insert(GetRHI()->Extensions.InstanceExtensions.end(), GlfwExtensions, GlfwExtensions + GlfwExtensionCount);

	VkApplicationInfo AppInfo{};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pApplicationName = "Rocket Engine Vulkan Application";
	AppInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	AppInfo.pEngineName = "No Engine";
	AppInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	AppInfo.apiVersion = VK_API_VERSION_1_3;

	VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo{};
	DebugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	DebugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	DebugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	DebugMessengerCreateInfo.pfnUserCallback = Utils::DebugUtilsMessengerCallback;

	VkInstanceCreateInfo InstanceCreateInfo{};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pApplicationInfo = &AppInfo;
	InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(GetRHI()->Extensions.InstanceExtensions.size());
	InstanceCreateInfo.ppEnabledExtensionNames = GetRHI()->Extensions.InstanceExtensions.data();
	InstanceCreateInfo.enabledLayerCount = GetRHI()->Extensions.ValidationLayerExtensions.size();
	InstanceCreateInfo.ppEnabledLayerNames = GetRHI()->Extensions.ValidationLayerExtensions.data();
	InstanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugMessengerCreateInfo;

	VkResult Result = vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize Vulkan instance.");

	Result = Utils::CreateDebugUtilsMessengerEXT(Instance, &DebugMessengerCreateInfo, nullptr, &DebugMessenger);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create debug messenger.");

	Result = glfwCreateWindowSurface(Instance, (GLFWwindow*)GetWindow()->GetNativeWindow(), nullptr, &Surface);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create Vulkan surface.");
}

void PVulkanInstance::Shutdown()
{
	vkDestroySurfaceKHR(Instance, Surface, nullptr);
	Utils::DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
	vkDestroyInstance(Instance, nullptr);
}

VkInstance PVulkanInstance::GetVkInstance() const
{
	return Instance;
}

VkSurfaceKHR PVulkanInstance::GetVkSurfaceKHR() const
{
	return Surface;
}