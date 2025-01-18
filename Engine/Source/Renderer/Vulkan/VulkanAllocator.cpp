#include "EnginePCH.h"
#include "VulkanAllocator.h"

#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanInstance.h"

void FVkAllocator::Init()
{
    VmaAllocatorCreateInfo AllocatorCreateInfo = {};
	AllocatorCreateInfo.physicalDevice = GetRHI()->GetDevice()->GetVkPhysicalDevice();
	AllocatorCreateInfo.device = GetRHI()->GetDevice()->GetVkDevice();
	AllocatorCreateInfo.instance = GetRHI()->GetInstance()->GetVkInstance();
	AllocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	VkResult Result = vmaCreateAllocator(&AllocatorCreateInfo, &MemoryAllocator);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create memory allocator.");
}

void FVkAllocator::Shutdown()
{
	vmaDestroyAllocator(MemoryAllocator);
}

VmaAllocator FVkAllocator::GetMemoryAllocator() const
{
    return MemoryAllocator;
}