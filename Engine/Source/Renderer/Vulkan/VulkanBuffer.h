#pragma once

#include "Renderer/Common/Buffer.h"

struct VkBuffer_T;
struct VmaAllocation_T;
struct VmaAllocationInfo;

typedef VkBuffer_T* VkBuffer;
typedef VmaAllocation_T* VmaAllocation;
typedef uint32_t VkBufferUsageFlags;

enum VmaMemoryUsage;

class PVulkanBuffer : public IBuffer
{
public:
    PVulkanBuffer(VkBufferUsageFlags InUsageFlags, VmaMemoryUsage InMemoryUsageFlags)
        : UsageFlags(InUsageFlags), MemoryUsageFlags(InMemoryUsageFlags)
    {
    }

    virtual void Allocate(size_t Size) override;
    virtual void Free() override;
    virtual void Submit(const void* Data, size_t Size, size_t Offset = 0) override;

    VkBuffer Buffer;
    VmaAllocation Allocation;
    VmaAllocationInfo AllocationInfo;

    VkBufferUsageFlags UsageFlags;
    VmaMemoryUsage MemoryUsageFlags;
};
