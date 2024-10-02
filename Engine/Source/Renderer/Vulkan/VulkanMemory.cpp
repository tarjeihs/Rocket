#include "EnginePCH.h"
#include "VulkanMemory.h"

#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanInstance.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"

void PVulkanMemory::Init()
{
	VmaAllocatorCreateInfo AllocatorCreateInfo = {};
	AllocatorCreateInfo.physicalDevice = GetRHI()->GetDevice()->GetVkPhysicalDevice();
	AllocatorCreateInfo.device = GetRHI()->GetDevice()->GetVkDevice();
	AllocatorCreateInfo.instance = GetRHI()->GetInstance()->GetVkInstance();
	AllocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	VkResult Result = vmaCreateAllocator(&AllocatorCreateInfo, &MemoryAllocator);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create memory allocator.");

	// Create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<SVulkanDescriptorPoolRatio> Sizes = { 
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
	};
	DescriptorPool = new PVulkanDescriptorPool();
	DescriptorPool->CreatePool(1024, Sizes);
}

void PVulkanMemory::Shutdown()
{
	for (PVulkanDescriptorSet* DescriptorSet : DescriptorSets)
	{
		DescriptorSet->DestroyDescriptorSet();
		delete DescriptorSet;
	}
	
	DescriptorPool->DestroyPool();
	delete DescriptorPool;
}

PVulkanDescriptorPool* PVulkanMemory::GetDescriptorPool() const
{
	return DescriptorPool;
}
