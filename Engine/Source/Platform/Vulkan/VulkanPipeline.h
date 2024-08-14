#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "VulkanTypes.h"

class PVulkanShader;

class PVulkanPipeline
{
public:
	using Super = PVulkanPipeline;

	PVulkanPipeline(VmaAllocator InAllocator)
		: Allocator(InAllocator)
	{
	}

	virtual void Init(VkDevice LogicalDevice, VkImageView DrawImageView, VkDescriptorPool DescriptorPool, VkFormat ImageFormat);
	virtual void Draw(VkCommandBuffer CommandBuffer, VkImageView DrawImageView, VkExtent2D DrawImageExtent) {}
	virtual void Destroy(VkDevice LogicalDevice);
	virtual void Free(VkDevice LogicalDevice);

protected:
	VkDescriptorSet			DescriptorSet		= VK_NULL_HANDLE;
	VkDescriptorSetLayout	DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout		PipelineLayout		= VK_NULL_HANDLE;
	VkPipeline				Pipeline			= VK_NULL_HANDLE;

protected:
	VmaAllocator Allocator;
};


// Used for rendering 3D models or meshes with vertex and fragment shaders. This is the standard pipeline for most 3D rendering tasks. (Vertex input, rasterization, color blending, and depth testing.)
class PVulkanColorGraphicsPipeline : public PVulkanPipeline
{
public:
	PVulkanColorGraphicsPipeline(VmaAllocator InAllocator)
		: Super(InAllocator)
	{
	}

	virtual void Init(VkDevice LogicalDevice, VkImageView DrawImageView, VkDescriptorPool DescriptorPool, VkFormat ImageFormat) override;
	virtual void Draw(VkCommandBuffer CommandBuffer, VkImageView DrawImageView, VkExtent2D DrawImageExtent) override;
	virtual void Destroy(VkDevice LogicalDevice) override;
	virtual void Free(VkDevice LogicalDevice) override;

	SMeshBuffer				MeshBuffer;

private:
	PVulkanShader* VertexShader = nullptr;
	PVulkanShader* FragmentShader = nullptr;
};

// Used for tasks like physics simulations, AI processing, image processing, or any general-purpose compute task that doesn't involve rendering. (Pure compute shader pipeline, no rasterization or vertex processing.)
class PVulkanStandardComputePipeline : public PVulkanPipeline
{
public:
	PVulkanStandardComputePipeline(VmaAllocator InAllocator)
		: Super(InAllocator)
	{
	}

	virtual void Init(VkDevice LogicalDevice, VkImageView DrawImageView, VkDescriptorPool DescriptorPool, VkFormat ImageFormat) override;
	virtual void Draw(VkCommandBuffer CommandBuffer, VkImageView DrawImageView, VkExtent2D DrawImageExtent) override;
	virtual void Destroy(VkDevice LogicalDevice) override;
	virtual void Free(VkDevice LogicalDevice) override;

private:
	PVulkanShader* ComputeShader = nullptr;
};