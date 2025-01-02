#include "EnginePCH.h"
#include "VulkanBuffer.h"

#include "Renderer/Vulkan/VulkanAllocator.h"

void FVkBuffer::Initialize(FVkBufferCreateInfo& CreateInfo)
{
	PROFILE_FUNC_SCOPE("FVkBuffer::Initialize")

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
	PROFILE_FUNC_SCOPE("FVkBuffer::Submit")

    void* MappedData;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.Allocation, &MappedData);
    memcpy(static_cast<uint8_t*>(MappedData) + Offset, Data, Size);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.Allocation);
}

void FVkBuffer::Free()
{
	PROFILE_FUNC_SCOPE("FVkBuffer::Free")

    vmaDestroyBuffer(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.Handle, Info.Allocation);
    Info.Handle = VK_NULL_HANDLE;
    Info.Allocation = VK_NULL_HANDLE;
}