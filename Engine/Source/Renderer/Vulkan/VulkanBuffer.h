#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "Renderer/Common/Buffer.h"

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
