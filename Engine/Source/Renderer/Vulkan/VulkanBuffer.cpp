#include "EnginePCH.h"
#include "VulkanBuffer.h"

#include "Renderer/Vulkan/VulkanAllocator.h"

namespace Utils
{
    VkBufferUsageFlags GetVkBufferUsageFlags(EBufferUsageFlag BufferUsageFlag)
    {
        VkBufferUsageFlags Flags = 0;

        if (HasFlag(BufferUsageFlag, EBufferUsageFlag::Storage)) Flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (HasFlag(BufferUsageFlag, EBufferUsageFlag::Uniform)) Flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (HasFlag(BufferUsageFlag, EBufferUsageFlag::Vertex)) Flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (HasFlag(BufferUsageFlag, EBufferUsageFlag::Index)) Flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (HasFlag(BufferUsageFlag, EBufferUsageFlag::Indirect)) Flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

        return Flags;
    }

    VkBufferUsageFlags GetVkBufferTransferFlags(EBufferTransferFlag BufferTransferFlag)
    {
        VkBufferUsageFlags Flags = 0;

        if (HasFlag(BufferTransferFlag, EBufferTransferFlag::Read)) Flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (HasFlag(BufferTransferFlag, EBufferTransferFlag::Write)) Flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        return Flags;
    }

    VmaMemoryUsage GetVmaMemoryUsageFlags(EBufferMemoryFlag BufferMemoryFlag)
    {
        switch (BufferMemoryFlag)
        {
            case EBufferMemoryFlag::Host:       return VMA_MEMORY_USAGE_CPU_ONLY;
            case EBufferMemoryFlag::HostToDevice:     return VMA_MEMORY_USAGE_CPU_TO_GPU;
            case EBufferMemoryFlag::Device:       return VMA_MEMORY_USAGE_GPU_ONLY;
            case EBufferMemoryFlag::DeviceToHost:     return VMA_MEMORY_USAGE_GPU_TO_CPU;
        }

        return VMA_MEMORY_USAGE_UNKNOWN;
    }
}

void FVkBuffer::Initialize(const FBufferCreateInfo& CreateInfo)
{
	PROFILE_FUNC_SCOPE("FVkBuffer::Initialize")

    VkBufferCreateInfo BufferCreateInfo = {};
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.pNext = nullptr;
    BufferCreateInfo.size = CreateInfo.Size;
    BufferCreateInfo.usage = Utils::GetVkBufferUsageFlags(CreateInfo.BufferUsageFlags) | Utils::GetVkBufferTransferFlags(CreateInfo.BufferTransferFlags);

    VmaAllocationCreateInfo AllocationCreateInfo = {};
    AllocationCreateInfo.usage = Utils::GetVmaMemoryUsageFlags(CreateInfo.MemoryUsageFlags);
    AllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkResult Result = vmaCreateBuffer(GetRHI()->GetAllocator()->GetMemoryAllocator(), &BufferCreateInfo, &AllocationCreateInfo, &Info.Handle, &Info.Allocation, &Info.AllocationInfo);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate buffer.");
}

void FVkBuffer::Shutdown()
{
	PROFILE_FUNC_SCOPE("FVkBuffer::Free")

    vmaDestroyBuffer(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.Handle, Info.Allocation);
    Info.Handle = VK_NULL_HANDLE;
    Info.Allocation = VK_NULL_HANDLE;
}

void FVkBuffer::Submit(const void* Data, size_t Size, size_t Offset)
{
	PROFILE_FUNC_SCOPE("FVkBuffer::Submit")

    void* MappedData;
    vmaMapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.Allocation, &MappedData);
    memcpy(static_cast<uint8_t*>(MappedData) + Offset, Data, Size);
    vmaUnmapMemory(GetRHI()->GetAllocator()->GetMemoryAllocator(), Info.Allocation);
}