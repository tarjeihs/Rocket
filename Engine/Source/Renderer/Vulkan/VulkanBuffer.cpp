#include "EnginePCH.h"
#include "VulkanBuffer.h"

#include "Core/Assert.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void SVulkanBuffer::Allocate(size_t Size)
{
    const PVulkanRHI* RHI = GetRHI<PVulkanRHI>();
    const PVulkanMemory* Memory = RHI->GetMemory();
    const PVulkanDevice* Device = RHI->GetDevice();

    VkBufferCreateInfo BufferCreateInfo{};
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.pNext = nullptr;
    BufferCreateInfo.size = Size;
    BufferCreateInfo.usage = UsageFlags;

    VmaAllocationCreateInfo AllocationCreateInfo{};
    AllocationCreateInfo.usage = MemoryUsageFlags;
    AllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkResult Result = vmaCreateBuffer(Memory->GetMemoryAllocator(), &BufferCreateInfo, &AllocationCreateInfo, &Buffer, &Allocation, &AllocationInfo);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate buffer.");
}

void SVulkanBuffer::Free()
{
    const PVulkanRHI* RHI = GetRHI<PVulkanRHI>();
    const PVulkanMemory* Memory = RHI->GetMemory();

    vmaDestroyBuffer(Memory->GetMemoryAllocator(), Buffer, Allocation);
    Buffer = VK_NULL_HANDLE;
    Allocation = VK_NULL_HANDLE;
}

void SVulkanBuffer::Update(const void *Data, size_t Size)
{
    const PVulkanRHI* RHI = GetRHI<PVulkanRHI>();
    const PVulkanMemory* Memory = RHI->GetMemory();

    void* MappedData;
    vmaMapMemory(Memory->GetMemoryAllocator(), Allocation, &MappedData);
    memcpy(MappedData, Data, Size);
    vmaUnmapMemory(RHI->GetMemory()->GetMemoryAllocator(), Allocation);
}