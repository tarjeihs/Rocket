#include "EnginePCH.h"
#include "VulkanDevice.h"

#include "Renderer/Vulkan/VulkanInstance.h"

void PVulkanDevice::Init()
{
	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(GetRHI()->GetInstance()->GetVkInstance(), &DeviceCount, 0);

	std::vector<VkPhysicalDevice> PhysicalDevices(DeviceCount);
	vkEnumeratePhysicalDevices(GetRHI()->GetInstance()->GetVkInstance(), &DeviceCount, PhysicalDevices.data());

	for (VkPhysicalDevice PhysicalDevice : PhysicalDevices)
	{
		uint32_t QueueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilies.data());
		
		for (uint32_t Index = 0; Index < QueueFamilyCount; Index++)
		{
			const VkQueueFamilyProperties& FamilyProperty = QueueFamilies[Index];
			VkBool32 PresentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, Index, GetRHI()->GetInstance()->GetVkSurfaceKHR(), &PresentSupport);
			
			if (FamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				GraphicsFamily = Index;
			}

			if (PresentSupport)
			{
				PresentFamily = Index;
			}

			if (GraphicsFamily.has_value() && PresentFamily.has_value())
			{
				break;
			}
		}

		uint32_t ExtensionCount;
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, nullptr);

		std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

		bool bExtensionSupport = std::all_of(GetRHI()->Extensions.PhysicalDeviceExtensions.begin(), GetRHI()->Extensions.PhysicalDeviceExtensions.end(), [&AvailableExtensions](const std::string& RequiredExtension)
		{
			return std::any_of(AvailableExtensions.begin(), AvailableExtensions.end(), [&RequiredExtension](const VkExtensionProperties& Extension)
			{
				return RequiredExtension == Extension.extensionName;
			});
		});

		if (bExtensionSupport)
		{
			uint32_t FormatCount;
			uint32_t PresentModeCount;
		
			vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, GetRHI()->GetInstance()->GetVkSurfaceKHR(), &FormatCount, nullptr);
			vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, GetRHI()->GetInstance()->GetVkSurfaceKHR(), &PresentModeCount, nullptr);

			if (FormatCount)
			{
				SurfaceFormats.resize(FormatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, GetRHI()->GetInstance()->GetVkSurfaceKHR(), &FormatCount, SurfaceFormats.data());
			}

			if (PresentModeCount)
			{
				PresentModes.resize(PresentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, GetRHI()->GetInstance()->GetVkSurfaceKHR(), &PresentModeCount, PresentModes.data());
			}

			if (!SurfaceFormats.empty() && !PresentModes.empty() && GraphicsFamily.has_value() && PresentFamily.has_value())
			{
				VkPhysicalDeviceProperties PhysicalDeviceProperties;
    			vkGetPhysicalDeviceProperties(PhysicalDevice, &PhysicalDeviceProperties);

				//RK_LOG_INFO("Using Physical Device: {}", PhysicalDeviceProperties.deviceName);
				PLogger::Log(ELogCategory ::LOG_INFO, "Using Physical Device: {}", PhysicalDeviceProperties.deviceName);

				GPU = PhysicalDevice;
				break;
			}
		}
	}

	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
	std::set<uint32_t> QueueFamilies { GraphicsFamily.value(), PresentFamily.value() };

	float QueuePriority = 1.0f;
	for (uint32_t QueueFamily : QueueFamilies)
	{
		VkDeviceQueueCreateInfo QueueFamilyCreateInfo{};
		QueueFamilyCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueFamilyCreateInfo.queueFamilyIndex = QueueFamily;
		QueueFamilyCreateInfo.queueCount = 1;
		QueueFamilyCreateInfo.pQueuePriorities = &QueuePriority;
		QueueCreateInfos.push_back(QueueFamilyCreateInfo);
	}

	// Vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features Features_1_3{};
	Features_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	Features_1_3.dynamicRendering = VK_TRUE;
	Features_1_3.synchronization2 = VK_TRUE;

	// Vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features Features_1_2{};
	Features_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	Features_1_2.bufferDeviceAddress = VK_TRUE;
	Features_1_2.descriptorIndexing = VK_TRUE;

	// Chain the features together
	Features_1_3.pNext = &Features_1_2;

	VkPhysicalDeviceFeatures DeviceFeatures{};
	DeviceFeatures.shaderInt64 = VK_TRUE;
	DeviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo DeviceCreateInfo{};
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
	DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
	DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
	DeviceCreateInfo.pNext = &Features_1_3;
	DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(GetRHI()->Extensions.PhysicalDeviceExtensions.size());
	DeviceCreateInfo.ppEnabledExtensionNames = GetRHI()->Extensions.PhysicalDeviceExtensions.data();
	DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(GetRHI()->Extensions.ValidationLayerExtensions.size());
	DeviceCreateInfo.ppEnabledLayerNames = GetRHI()->Extensions.ValidationLayerExtensions.data();

	VkResult Result = vkCreateDevice(GPU, &DeviceCreateInfo, nullptr, &Device);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create logical device.");

	vkGetDeviceQueue(Device, GraphicsFamily.value(), 0, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentFamily.value(), 0, &PresentQueue);
}

void PVulkanDevice::Shutdown()
{
	vkDestroyDevice(Device, nullptr);
}

VkPhysicalDevice PVulkanDevice::GetVkPhysicalDevice() const
{
	return GPU;
}

VkDevice PVulkanDevice::GetVkDevice() const
{
	return Device;
}

VkQueue PVulkanDevice::GetGraphicsQueue() const
{
	return GraphicsQueue;
}

VkQueue PVulkanDevice::GetPresentQueue() const
{
	return PresentQueue;
}

std::optional<uint32_t> PVulkanDevice::GetGraphicsFamilyIndex() const
{
	return GraphicsFamily;
}

std::optional<uint32_t> PVulkanDevice::GetPresentFamilyIndex() const
{
	return PresentFamily;
}

const std::vector<VkSurfaceFormatKHR>& PVulkanDevice::GetSurfaceFormats() const
{
	return SurfaceFormats;
}

const std::vector<VkPresentModeKHR>& PVulkanDevice::GetPresentModes() const
{
	return PresentModes;
}

VkSurfaceCapabilitiesKHR PVulkanDevice::GetSurfaceCapabilities() const
{
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GPU, GetRHI()->GetInstance()->GetVkSurfaceKHR(), &SurfaceCapabilities);
	return SurfaceCapabilities;
}

VkPhysicalDeviceProperties PVulkanDevice::GetPhysicalDeviceProperties() const
{
	VkPhysicalDeviceProperties PhysicalDeviceProperties;
	vkGetPhysicalDeviceProperties(GPU, &PhysicalDeviceProperties);
	return PhysicalDeviceProperties;
}
