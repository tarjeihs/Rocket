#include "EnginePCH.h"
#include "VulkanBuffer.h"

#include "Renderer/Vulkan/VulkanSceneRenderer.h"
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

    VkResult Result = vmaCreateBuffer(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), &BufferCreateInfo, &AllocationCreateInfo, &Buffer, &Allocation, &AllocationInfo);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate buffer.");
}

void PVulkanBuffer::Free()
{
    vmaDestroyBuffer(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), Buffer, Allocation);
    Buffer = VK_NULL_HANDLE;
    Allocation = VK_NULL_HANDLE;
}

void PVulkanBuffer::Submit(const void* Data, size_t Size, size_t Offset)
{
    void* MappedData;
    vmaMapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), Allocation, &MappedData);
    memcpy(static_cast<uint8_t*>(MappedData) + Offset, Data, Size);
    vmaUnmapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), Allocation);
}