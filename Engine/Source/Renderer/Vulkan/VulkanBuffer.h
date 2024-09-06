#pragma once

#include "Renderer/Common/Buffer.h"

struct VkBuffer_T;
struct VmaAllocation_T;
struct VmaAllocationInfo;

typedef VkBuffer_T* VkBuffer;
typedef VmaAllocation_T* VmaAllocation;
typedef uint32_t VkBufferUsageFlags;

enum VmaMemoryUsage;

struct SVulkanBuffer : public IBuffer
{
    SVulkanBuffer(VkBufferUsageFlags InUsageFlags, VmaMemoryUsage InMemoryUsageFlags)
        : UsageFlags(InUsageFlags), MemoryUsageFlags(InMemoryUsageFlags)
    {
    }

    virtual void Allocate(size_t Size) override;
    virtual void Free() override;
    virtual void Update(const void* Data, size_t Size) override;

    VkBuffer Buffer;
    VmaAllocation Allocation;
    VmaAllocationInfo AllocationInfo;

    VkBufferUsageFlags UsageFlags;
    VmaMemoryUsage MemoryUsageFlags;
};
