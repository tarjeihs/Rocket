#pragma once

#include <glm/fwd.hpp>
#include <glm/matrix.hpp>

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
	void Init();
	void Shutdown();
};