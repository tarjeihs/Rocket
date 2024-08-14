#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>

struct SVulkanImage
{
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
	VkExtent3D imageExtent;
	VkFormat imageFormat;
};

struct SVertex
{
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
};

// A generic GPU buffer (eg. vertex buffer, index buffer, uniform buffer, staging buffer)
struct SBuffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo info;
};

// holds the resources needed for a mesh
struct SMeshBuffer
{
	SBuffer indexBuffer;
	SBuffer vertexBuffer;
	VkDeviceAddress vertexBufferAddress;
};

// push constants for our mesh object draws
struct SMeshPushConstant
{
	glm::mat4 worldMatrix;
	VkDeviceAddress vertexBuffer;
};

struct SComputePushConstant
{
	glm::vec4 data1;
	glm::vec4 data2;
	glm::vec4 data3;
	glm::vec4 data4;
};