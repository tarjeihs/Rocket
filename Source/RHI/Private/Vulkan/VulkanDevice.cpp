#include "RocketPCH.h"
#include "RHI/Private/Vulkan/VulkanDevice.h"

#include "RHI/Private/Vulkan/VulkanQueue.h"
#include "RHI/Public/Vulkan/VulkanRHIMinimal.h"

FVulkanDevice::FVulkanDevice(VkPhysicalDevice InPhysicalDevice)
    : PhysicalDevice(InPhysicalDevice)
{
	Initialize();
}

FVulkanDevice::~FVulkanDevice()
{
	Shutdown();
}

void FVulkanDevice::WaitUntilIdle() const
{
	vkDeviceWaitIdle(Device);
}

void FVulkanDevice::Initialize()
{
    uint32_t QueueFamilyCount = 0;
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
	Features_1_2.timelineSemaphore = VK_TRUE;

	// Chain the features together
	Features_1_3.pNext = &Features_1_2;

	VkPhysicalDeviceFeatures DeviceFeatures = {};
	DeviceFeatures.shaderInt64 = VK_TRUE;
	DeviceFeatures.samplerAnisotropy = VK_TRUE;
	DeviceFeatures.multiDrawIndirect = VK_TRUE;

	VkDeviceCreateInfo DeviceCreateInfo = {};
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
	DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
	DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
	DeviceCreateInfo.pNext = &Features_1_3;
	DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(GetVulkanRHIMinimal()->GetPhysicalDeviceExtensions().size());
	DeviceCreateInfo.ppEnabledExtensionNames = GetVulkanRHIMinimal()->GetPhysicalDeviceExtensions().data();
	DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(GetVulkanRHIMinimal()->GetValidationExtensions().size());
	DeviceCreateInfo.ppEnabledLayerNames = GetVulkanRHIMinimal()->GetValidationExtensions().data();
	VkResult Result = vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &Device);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize logical device.");

	VmaAllocatorCreateInfo AllocatorCreateInfo = {};
	AllocatorCreateInfo.physicalDevice = PhysicalDevice;
	AllocatorCreateInfo.device = Device;
	AllocatorCreateInfo.instance = GetVulkanRHIMinimal()->RHIGetVkInstance();
	AllocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	Result = vmaCreateAllocator(&AllocatorCreateInfo, &Allocator);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize memory allocator.");

	for (uint32_t Index = 0; Index < QueueFamilyCount; Index++)
	{
        if ((QueueFamiliesProperties[Index].queueFlags & ~(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT)) != 0)
        {
            continue;
        }

        Queues.push_back(std::make_unique<FVulkanQueue>(*this, QueueFamiliesProperties[Index].queueFlags, false, Index, 1));
    }
}

void FVulkanDevice::Shutdown()
{
	Queues.clear();

	vmaDestroyAllocator(Allocator);
	vkDestroyDevice(Device, nullptr);
}

FVulkanQueue* FVulkanDevice::GetQueue(VkQueueFlags QueueFlags) const
{
    for (const std::unique_ptr<FVulkanQueue>& Queue : Queues)
    {
        if ((Queue->GetQueueFlags() & QueueFlags) == QueueFlags)
        {
            return Queue.get();
        }
    }
	return nullptr;
}

FVulkanQueue* FVulkanDevice::GetGraphicsQueue() const
{
    return GetQueue(VK_QUEUE_GRAPHICS_BIT);
}

FVulkanQueue* FVulkanDevice::GetComputeQueue() const
{
    return GetQueue((VK_QUEUE_COMPUTE_BIT) & ~(VK_QUEUE_GRAPHICS_BIT));
}

FVulkanQueue* FVulkanDevice::GetTransferQueue() const
{
    return GetQueue((VK_QUEUE_TRANSFER_BIT) & ~(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT));
}

FVulkanQueue* FVulkanDevice::GetPresentQueue() const
{
    return GetQueue(VK_QUEUE_GRAPHICS_BIT);
}