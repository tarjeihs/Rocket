#define VMA_IMPLEMENTATION

#include "VulkanMemory.h"

#include "Core/Assert.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanInstance.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"

void PVulkanMemory::Init()
{
	VmaAllocatorCreateInfo AllocatorCreateInfo = {};
	AllocatorCreateInfo.physicalDevice = RHI->GetDevice()->GetVkPhysicalDevice();
	AllocatorCreateInfo.device = RHI->GetDevice()->GetVkDevice();
	AllocatorCreateInfo.instance = RHI->GetInstance()->GetVkInstance();
	AllocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	VkResult Result = vmaCreateAllocator(&AllocatorCreateInfo, &MemoryAllocator);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create memory allocator.");

	// Create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<SVulkanDescriptorPoolRatio> Sizes = { { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 } };
	DescriptorPool = new PVulkanDescriptorPool();
	DescriptorPool->CreatePool(RHI, 10, Sizes);
}

void PVulkanMemory::Shutdown()
{
	DescriptorPool->DestroyPool(RHI);
	delete DescriptorPool;

	vmaDestroyAllocator(MemoryAllocator);
}

SBuffer* PVulkanMemory::CreateBuffer(size_t Size, VmaMemoryUsage MemoryUsage, VkBufferUsageFlags UsageFlags)
{
	VkBufferCreateInfo BufferCreateInfo{};
	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.pNext = nullptr;
	BufferCreateInfo.size = Size;
	BufferCreateInfo.usage = UsageFlags;

	VmaAllocationCreateInfo AllocationCreateInfo{};
	AllocationCreateInfo.usage = MemoryUsage;
	AllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	auto* Buffer = new SBuffer();

	VkResult Result = vmaCreateBuffer(MemoryAllocator, &BufferCreateInfo, &AllocationCreateInfo, &Buffer->Buffer, &Buffer->Allocation, &Buffer->AllocationInfo);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate buffer.");

	return Buffer;
}

void PVulkanMemory::FreeBuffer(SBuffer* Buffer)
{
	vmaDestroyBuffer(MemoryAllocator, Buffer->Buffer, Buffer->Allocation);
}

void PVulkanMemory::UploadBuffer(SBuffer* Buffer, void* Data, size_t Size)
{
	void* MappedData;
	vmaMapMemory(MemoryAllocator, Buffer->Allocation, &MappedData);
	memcpy(MappedData, Data, Size);
	vmaUnmapMemory(RHI->GetMemory()->GetMemoryAllocator(), Buffer->Allocation);
}

VmaAllocator PVulkanMemory::GetMemoryAllocator() const
{
	return MemoryAllocator;
}

PVulkanDescriptorPool* PVulkanMemory::GetDescriptorPool() const
{
	return DescriptorPool;
}
