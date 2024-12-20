#include "EnginePCH.h"
#include "VulkanBuffer.h"

#include "Renderer/Vulkan/VulkanAllocator.h"

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

    VkResult Result = vmaCreateBuffer(GetRHI()->GetAllocator()->GetMemoryAllocator(), &BufferCreateInfo, &AllocationCreateInfo, &Buffer, &Allocation, &AllocationInfo);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate buffer.");
}

void PVulkanBuffer::Free()
{
    vmaDestroyBuffer(GetRHI()->GetAllocator()->GetMemoryAllocator(), Buffer, Allocation);
    Buffer = VK_NULL_HANDLE;
    Allocation = VK_NULL_HANDLE;
}

void PVulkanBuffer::Submit(const void* Data, size_t Size, size_t Offset)
{
    void* MappedData;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Allocation, &MappedData);
    memcpy(static_cast<uint8_t*>(MappedData) + Offset, Data, Size);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Allocation);
}

// FVkBuffer

void FVkBuffer::Initialize(FVkBufferCreateInfo& CreateInfo)
{
    VkBufferCreateInfo BufferCreateInfo{};
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.pNext = nullptr;
    BufferCreateInfo.size = CreateInfo.Size;
    BufferCreateInfo.usage = CreateInfo.UsageFlags;

    VmaAllocationCreateInfo AllocationCreateInfo{};
    AllocationCreateInfo.usage = CreateInfo.MemoryUsageFlags;
    AllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkResult Result = vmaCreateBuffer(GetRHI()->GetAllocator()->GetMemoryAllocator(), &BufferCreateInfo, &AllocationCreateInfo, &Info.Handle, &Info.Allocation, &Info.AllocationInfo);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate buffer.");
}

void FVkBuffer::Submit(const void* Data, size_t Size, size_t Offset)
{
    RK_ASSERT((Offset % 16) == 0 && "Offset is not 16-byte aligned!");
    void* MappedData;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.Allocation, &MappedData);
    memcpy(static_cast<uint8_t*>(MappedData) + Offset, Data, Size);
    //vmaFlushAllocation(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), Info.Allocation, Offset, Size); // Ensure visibility
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.Allocation);
}

void FVkBuffer::Free()
{
    vmaDestroyBuffer(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.Handle, Info.Allocation);
    Info.Handle = VK_NULL_HANDLE;
    Info.Allocation = VK_NULL_HANDLE;
}