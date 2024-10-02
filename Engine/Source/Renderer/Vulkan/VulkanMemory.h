#pragma once

#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>

class PVulkanDescriptorPool;
struct VmaAllocator_T;

typedef VmaAllocator_T* VmaAllocator;
typedef uint64_t VkDeviceAddress;

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
	PVulkanMemory()
	{
		MemoryAllocator = nullptr;
		DescriptorPool = nullptr;
	}

	void Init();
	void Shutdown();

	PVulkanDescriptorPool* GetDescriptorPool() const;

protected:
	VmaAllocator MemoryAllocator;

	PVulkanDescriptorPool* DescriptorPool;

public:
	std::vector<PVulkanDescriptorSet*> DescriptorSets;
};