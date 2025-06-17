#include "VulkanDevice.h"

#include "VulkanViewport.h"
#include "VulkanQueue.h"
#include "VulkanRHI.h"

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

void * CVulkanDevice::GetNativeDevice() const
{
	return PhysicalDevice;
}

void * CVulkanDevice::GetNativeDriver() const
{
	return LogicalDevice;
}

void * CVulkanDevice::GetNativeInstance() const
{
	return Instance;
}

void CVulkanDevice::WaitUntilIdle() const
{
	vkDeviceWaitIdle(LogicalDevice);
}

void CVulkanDevice::Initialize()
{
	InstanceExtensions.push_back("VK_EXT_swapchain_colorspace");
	PhysicalDeviceExtensions.push_back("VK_KHR_swapchain");

#if DEBUG_VALIDATION
	InstanceExtensions.push_back("VK_EXT_debug_utils");
	ValidationLayerExtensions.push_back("VK_LAYER_KHRONOS_validation");
#endif

	// GLFW required Vulkan extensions
	uint32_t GlfwExtensionCount = 0;
	const char** GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);
	InstanceExtensions.insert(InstanceExtensions.end(), GlfwExtensions, GlfwExtensions + GlfwExtensionCount);

	VkApplicationInfo ApplicationInfo = {};
	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pApplicationName = "Rocket";
	ApplicationInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	ApplicationInfo.pEngineName = "No Engine";
	ApplicationInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

	VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo{};
	DebugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	DebugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	DebugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	DebugMessengerCreateInfo.pfnUserCallback = ExecDebugCallback;

	VkInstanceCreateInfo InstanceCreateInfo = {};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
	InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
	InstanceCreateInfo.ppEnabledExtensionNames = InstanceExtensions.data();
	InstanceCreateInfo.enabledLayerCount = ValidationLayerExtensions.size();
	InstanceCreateInfo.ppEnabledLayerNames = ValidationLayerExtensions.data();
	InstanceCreateInfo.pNext = &DebugMessengerCreateInfo;

	VkResult Result = vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize Vulkan instance.");

#if DEBUG_VALIDATION
	Result = BindDebugCallback(Instance, &DebugMessengerCreateInfo, nullptr, &DebugCallback);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create debug callback.");
#endif

	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);

	std::vector<VkPhysicalDevice> AllPhysicalDevices(DeviceCount);
	vkEnumeratePhysicalDevices(Instance, &DeviceCount, AllPhysicalDevices.data());

	std::vector<VkPhysicalDevice> DiscreteDevices;
	std::vector<VkPhysicalDevice> IntegratedDevices;
	std::vector<VkPhysicalDevice> VirtualDevices;

	for (VkPhysicalDevice GPU : AllPhysicalDevices)
	{
		VkPhysicalDeviceProperties GPUProperties;
		vkGetPhysicalDeviceProperties(GPU, &GPUProperties);

		if (GPUProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) DiscreteDevices.push_back(GPU);
		if (GPUProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) IntegratedDevices.push_back(GPU);
		if (GPUProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) VirtualDevices.push_back(GPU);
	}

	std::vector<VkPhysicalDevice> AvailablePhysicalDevices;
	AvailablePhysicalDevices.insert(AvailablePhysicalDevices.end(), DiscreteDevices.begin(), DiscreteDevices.end());
	AvailablePhysicalDevices.insert(AvailablePhysicalDevices.end(), IntegratedDevices.begin(), IntegratedDevices.end());
	AvailablePhysicalDevices.insert(AvailablePhysicalDevices.end(), VirtualDevices.begin(), VirtualDevices.end());

	for (VkPhysicalDevice CurrentPhysicalDevice : AvailablePhysicalDevices)
	{
		uint32_t ExtensionCount;
		vkEnumerateDeviceExtensionProperties(CurrentPhysicalDevice, nullptr, &ExtensionCount, nullptr);

		std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
		vkEnumerateDeviceExtensionProperties(CurrentPhysicalDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

		std::unordered_set<std::string_view> Filter;
		Filter.insert(PhysicalDeviceExtensions.begin(), PhysicalDeviceExtensions.end());

		for (VkExtensionProperties Extension : AvailableExtensions)
		{
			if (Filter.contains(Extension.extensionName))
			{
				Filter.erase(Extension.extensionName);
			}
		}

		if (Filter.empty())
		{
			PhysicalDevice = CurrentPhysicalDevice;
			break;
		}
	}

	uint32 QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamiliesProperties(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamiliesProperties.data());

	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;

	for (int Index = 0; Index < QueueFamilyCount; Index++)
	{
		float QueuePriority = 1.0f;
		VkDeviceQueueCreateInfo QueueFamilyCreateInfo = {};
		QueueFamilyCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueFamilyCreateInfo.queueCount = 1;
		QueueFamilyCreateInfo.queueFamilyIndex = Index;
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

	Result = vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &LogicalDevice);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize logical device.");

	VmaAllocatorCreateInfo AllocatorCreateInfo = {};
	AllocatorCreateInfo.physicalDevice = PhysicalDevice;
	AllocatorCreateInfo.device = LogicalDevice;
	AllocatorCreateInfo.instance = Instance;
	AllocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	Result = vmaCreateAllocator(&AllocatorCreateInfo, &Allocator);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize memory allocator.");

	for (uint32 Index = 0; Index < QueueFamilyCount; Index++)
	{
		if ((QueueFamiliesProperties[Index].queueFlags & 0xF) == 0xF) // Raster + Compute + Transfer + Sparse
		{
			Queue[ERHIQueueClass::Graphics].Capabilities.Engine = Index;
			vkGetDeviceQueue(GetLogicalDevice(), Index, 0, &Queue[ERHIQueueClass::Graphics].Queue);
		}
		else if ((QueueFamiliesProperties[Index].queueFlags & 0xE) == 0xE) // Compute + Transfer + Sparse
		{
			Queue[ERHIQueueClass::Compute].Capabilities.Engine = Index;
			vkGetDeviceQueue(GetLogicalDevice(), Index, 0, &Queue[ERHIQueueClass::Compute].Queue);
		}
		else if ((QueueFamiliesProperties[Index].queueFlags & 0xC) == 0xC) // Transfer + Sparse
		{
			Queue[ERHIQueueClass::Transfer].Capabilities.Engine = Index;
			vkGetDeviceQueue(GetLogicalDevice(), Index, 0, &Queue[ERHIQueueClass::Transfer].Queue);
		}
	}
}

void CVulkanDevice::Shutdown()
{
	vmaDestroyAllocator(Allocator);
	vkDestroyDevice(LogicalDevice, nullptr);

#if DEBUG_VALIDATION
	UnbindDebugCallback(Instance, DebugCallback, nullptr);
#endif

	vkDestroyInstance(Instance, nullptr);

	PhysicalDevice = nullptr;
	LogicalDevice = nullptr;
	DebugCallback = nullptr;
	Instance = nullptr;
}

IRHIQueue * CVulkanDevice::GetGraphicsQueue()
{
	return &Queue.at(ERHIQueueClass::Graphics);
}

IRHIQueue * CVulkanDevice::GetComputeQueue()
{
	return &Queue.at(ERHIQueueClass::Compute);
}

IRHIQueue * CVulkanDevice::GetTransferQueue()
{
	return &Queue.at(ERHIQueueClass::Transfer);
}

IRHIQueue * CVulkanDevice::GetPresentQueue()
{
	return &Queue.at(ERHIQueueClass::Graphics);
}

VkInstance CVulkanDevice::GetRHIVulkanInstance() const
{
	return Instance;
}

VkPhysicalDevice CVulkanDevice::GetRHIVulkanPhysicalDevice() const
{
	return PhysicalDevice;
}

VkDevice CVulkanDevice::GetLogicalDevice() const
{
	return LogicalDevice;
}

VmaAllocator CVulkanDevice::GetAllocator() const
{
	return Allocator;
}
