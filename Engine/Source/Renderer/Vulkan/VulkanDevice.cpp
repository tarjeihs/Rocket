#include "VulkanDevice.h"

#include "VulkanViewport.h"

static VkResult BindDebugCallback(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* CreateInfo, const VkAllocationCallbacks* Allocator, VkDebugUtilsMessengerEXT* DebugCallback)
{
	auto Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
	if (Func != nullptr)
	{
		return Func(Instance, CreateInfo, Allocator, DebugCallback);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void UnbindDebugCallback(VkInstance Instance, VkDebugUtilsMessengerEXT DebugCallback, const VkAllocationCallbacks* Allocator)
{
	auto Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
	if (Func != nullptr)
	{
		Func(Instance, DebugCallback, Allocator);
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL ExecDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
	switch (Severity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	{ RK_LOG_DEBUG("[VL] {}", CallbackData->pMessage); break; }
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		{ RK_LOG_INFO("[VL] {}", CallbackData->pMessage);   break; }
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	{ RK_LOG_WARNING("[VL] {}", CallbackData->pMessage); break; }
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		{ RK_LOG_ERROR("[VL] {}", CallbackData->pMessage);  break; }
		default: break;
	}
	return VK_FALSE;
}

void * CVulkanDevice::GetNativeInstance() const
{
	return Instance;
}

void CVulkanDevice::WaitUntilIdle() const
{
	vkDeviceWaitIdle(LogicalDevice);
}

void CVulkanDevice::CreateInstance()
{
	InstanceExtensions.push_back("VK_EXT_swapchain_colorspace");
	PhysicalDeviceExtensions.push_back("VK_KHR_swapchain");

#if VALIDATION_LAYER
	InstanceExtensions.push_back("VK_EXT_debug_utils");
	ValidationLayerExtensions.push_back("VK_LAYER_KHRONOS_validation");
#endif

	// GLFW required Vulkan extensions
	uint32_t GlfwExtensionCount = 0;
	const char** GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);
	InstanceExtensions.insert(InstanceExtensions.end(), GlfwExtensions, GlfwExtensions + GlfwExtensionCount);

	VkApplicationInfo ApplicationInfo = {};
	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pApplicationName = "Rocket Engine";
	ApplicationInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	ApplicationInfo.pEngineName = "No Engine";
	ApplicationInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

	VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo{};
	DebugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	DebugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	DebugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	DebugMessengerCreateInfo.pfnUserCallback = ExecDebugCallback;

	VkInstanceCreateInfo InstanceCreateInfo{};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
	InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
	InstanceCreateInfo.ppEnabledExtensionNames = InstanceExtensions.data();
	InstanceCreateInfo.enabledLayerCount = ValidationLayerExtensions.size();
	InstanceCreateInfo.ppEnabledLayerNames = ValidationLayerExtensions.data();
	InstanceCreateInfo.pNext = &DebugMessengerCreateInfo;

	VkResult Result = vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize Vulkan instance.");

#if VALIDATION_LAYER
	Result = BindDebugCallback(Instance, &DebugMessengerCreateInfo, nullptr, &DebugCallback);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create debug callback.");
#endif
}

void CVulkanDevice::CreateDevice(CVulkanViewport* Viewport)
{
	std::optional<uint32> GraphicsQueueFamily;
	std::optional<uint32> PresentQueueFamily;

	std::vector<VkSurfaceFormatKHR> SurfaceFormats;
	std::vector<VkPresentModeKHR> PresentModes;

	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);

	std::vector<VkPhysicalDevice> PhysicalDevices(DeviceCount);
	vkEnumeratePhysicalDevices(Instance, &DeviceCount, PhysicalDevices.data());

	for (VkPhysicalDevice GPU : PhysicalDevices)
	{
		GetGraphicsQueueFamily(GPU, Viewport->Surface, GraphicsQueueFamily);
		GetPresentQueueFamily(GPU, Viewport->Surface, PresentQueueFamily);

		Viewport->GetSurfaceFormats(GPU, SurfaceFormats);
		Viewport->GetPresentModes(GPU, PresentModes);

		if (!SurfaceFormats.empty() && !PresentModes.empty() && GraphicsQueueFamily.has_value() && PresentQueueFamily.has_value())
		{
			VkPhysicalDeviceProperties PhysicalDeviceProperties;
			vkGetPhysicalDeviceProperties(GPU, &PhysicalDeviceProperties);

			PhysicalDevice = GPU;
			break;
		}
	}

	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
	std::set<uint32_t> QueueFamilies = { GraphicsQueueFamily.value(), PresentQueueFamily.value() };

	float QueuePriority = 1.0f;
	for (uint32_t QueueFamily : QueueFamilies)
	{
		VkDeviceQueueCreateInfo QueueFamilyCreateInfo = {};
		QueueFamilyCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueFamilyCreateInfo.queueFamilyIndex = QueueFamily;
		QueueFamilyCreateInfo.queueCount = 1;
		QueueFamilyCreateInfo.pQueuePriorities = &QueuePriority;
		QueueCreateInfos.push_back(QueueFamilyCreateInfo);
	}

	// Vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features Features_1_3 = {};
	Features_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	Features_1_3.dynamicRendering = VK_TRUE;
	Features_1_3.synchronization2 = VK_TRUE;

	// Vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features Features_1_2 = {};
	Features_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	Features_1_2.bufferDeviceAddress = VK_TRUE;
	Features_1_2.bufferDeviceAddressCaptureReplay = VK_TRUE;
	Features_1_2.descriptorIndexing = VK_TRUE;
	Features_1_2.runtimeDescriptorArray = VK_TRUE;
	Features_1_2.descriptorBindingPartiallyBound = VK_TRUE;
	Features_1_2.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	Features_1_2.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
	Features_1_2.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;

	// Chain the features together
	Features_1_3.pNext = &Features_1_2;

	VkPhysicalDeviceFeatures DeviceFeatures = {};
	DeviceFeatures.shaderInt64 = VK_TRUE;
	DeviceFeatures.samplerAnisotropy = VK_TRUE;
	DeviceFeatures.multiDrawIndirect = VK_TRUE;

	VkDeviceCreateInfo DeviceCreateInfo{};
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
	DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
	DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
	DeviceCreateInfo.pNext = &Features_1_3;
	DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(PhysicalDeviceExtensions.size());
	DeviceCreateInfo.ppEnabledExtensionNames = PhysicalDeviceExtensions.data();
	DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayerExtensions.size());
	DeviceCreateInfo.ppEnabledLayerNames = ValidationLayerExtensions.data();

	VkResult Result = vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &LogicalDevice);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize logical device.");

	VmaAllocatorCreateInfo AllocatorCreateInfo = {};
	AllocatorCreateInfo.physicalDevice = PhysicalDevice;
	AllocatorCreateInfo.device = LogicalDevice;
	AllocatorCreateInfo.instance = Instance;
	AllocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	Result = vmaCreateAllocator(&AllocatorCreateInfo, &Allocator);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize memory allocator.");
}

void CVulkanDevice::FreeInstance()
{
#if VALIDATION_LAYER
	UnbindDebugCallback(Instance, DebugCallback, nullptr);
#endif

	vkDestroyInstance(Instance, nullptr);

	Instance = nullptr;
	DebugCallback = nullptr;
}

void CVulkanDevice::FreeDevice()
{
	vmaDestroyAllocator(Allocator);
	vkDestroyDevice(LogicalDevice, nullptr);

	PhysicalDevice = nullptr;
	LogicalDevice = nullptr;
}

bool CVulkanDevice::GetGraphicsQueueFamily(VkPhysicalDevice InPhysicalDevice, VkSurfaceKHR Surface, std::optional<uint32_t> &OutGraphicsQueueFamily)
{
	uint32 QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueFamilyCount, QueueFamilies.data());

	for (uint32 Index = 0; Index < QueueFamilyCount; Index++)
	{
		if (QueueFamilies[Index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			OutGraphicsQueueFamily = Index;
			RK_LOG_INFO("An appropriate graphics queue family has been found.");
			return true;
		}
	}

	return false;
}

bool CVulkanDevice::GetPresentQueueFamily(VkPhysicalDevice InPhysicalDevice, VkSurfaceKHR Surface, std::optional<uint32_t> &OutPresentQueueFamily)
{
	uint32 QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueFamilyCount, QueueFamilies.data());

	for (uint32 Index = 0; Index < QueueFamilyCount; ++Index)
	{
		VkBool32 PresentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(InPhysicalDevice, Index, Surface, &PresentSupport);

		if (PresentSupport)
		{
			OutPresentQueueFamily = Index;
			RK_LOG_INFO("An appropriate present queue family has been found.");
			return true;
		}
	}

	return false;
}

void CVulkanDevice::GetGraphicsQueue(VkSurfaceKHR Surface, VkQueue &OutGraphicsQueue)
{
	RK_ASSERT(PhysicalDevice != nullptr, "Physical device cannot be null.");

	std::optional<uint32_t> GraphicsQueueFamily;
	bool Success = GetGraphicsQueueFamily(PhysicalDevice, Surface, GraphicsQueueFamily);

	if (Success)
	{
		vkGetDeviceQueue(LogicalDevice, GraphicsQueueFamily.value(), 0, &OutGraphicsQueue);
	}
}

void CVulkanDevice::GetPresentQueue(VkSurfaceKHR Surface, VkQueue &OutPresentQueue)
{
	RK_ASSERT(PhysicalDevice != nullptr, "Physical device cannot be null.");

	std::optional<uint32_t> PresentQueueFamily;
	bool Success = GetPresentQueueFamily(PhysicalDevice, Surface, PresentQueueFamily);

	if (Success)
	{
		vkGetDeviceQueue(LogicalDevice, PresentQueueFamily.value(), 0, &OutPresentQueue);
	}
}
