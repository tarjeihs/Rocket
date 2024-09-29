#include "EnginePCH.h"
#include "VulkanBuffer.h"

#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanBuffer::Allocate(size_t Size)
{
    VkBufferCreateInfo BufferCreateInfo{};
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.pNext = nullptr;
    BufferCreateInfo.size = Size;
    BufferCreateInfo.usage = UsageFlags;

    VmaAllocationCreateInfo AllocationCreateInfo{};
    AllocationCreateInfo.usage = MemoryUsageFlags;
    AllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkResult Result = vmaCreateBuffer(GetRHI()->GetMemory()->GetMemoryAllocator(), &BufferCreateInfo, &AllocationCreateInfo, &Buffer, &Allocation, &AllocationInfo);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate buffer.");
}

void PVulkanBuffer::Free()
{
    vmaDestroyBuffer(GetRHI()->GetMemory()->GetMemoryAllocator(), Buffer, Allocation);
    Buffer = VK_NULL_HANDLE;
    Allocation = VK_NULL_HANDLE;
}

void PVulkanBuffer::Submit(const void* Data, size_t Size)
{
    void* MappedData;
    vmaMapMemory(GetRHI()->GetMemory()->GetMemoryAllocator(), Allocation, &MappedData);
    memcpy(MappedData, Data, Size);
    vmaUnmapMemory(GetRHI()->GetMemory()->GetMemoryAllocator(), Allocation);
}