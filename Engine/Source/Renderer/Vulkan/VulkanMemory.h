#pragma once

#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

class PVulkanDescriptorPool;
class PVulkanRHI;

struct SBuffer
{
	VkBuffer Buffer;
	VmaAllocation Allocation;
	VmaAllocationInfo AllocationInfo;
};

struct SShaderStorageBufferObject
{
	alignas(16) glm::mat4 ModelMatrix;
	alignas(16) glm::mat4 NormalMatrix;
};

struct SUniformBufferObject
{
	glm::mat4 WorldMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
};

struct SUInt64PointerPushConstant
{
	VkDeviceAddress DeviceAddress;
	uint32_t ObjectId;
};

class PVulkanMemory
{
public:
	PVulkanMemory(PVulkanRHI* InRHI)
		: RHI(InRHI)
	{
		MemoryAllocator = nullptr;
		DescriptorPool = nullptr;
	}

	void Init();
	void Shutdown();

	void AllocateBuffer(SBuffer*& Buffer, size_t Size, VmaMemoryUsage MemoryUsage, VkBufferUsageFlags UsageFlags);
	void FreeBuffer(SBuffer* Buffer);
	void UploadBuffer(SBuffer* Buffer, void* Data, size_t Size);

	VmaAllocator GetMemoryAllocator() const;
	PVulkanDescriptorPool* GetDescriptorPool() const;

protected:
	VmaAllocator MemoryAllocator;
	PVulkanDescriptorPool* DescriptorPool;

private:
	PVulkanRHI* RHI;
};