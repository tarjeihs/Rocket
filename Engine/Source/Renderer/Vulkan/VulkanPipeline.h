#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "Math/Transform.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanMemory.h"

class IMesh;
class PVulkanDescriptorSet;
class PVulkanDescriptorSetLayout;
class PVulkanShader;
class PVulkanCommandBuffer;
struct SBuffer;
struct SUniformBufferObject;

class PVulkanPipelineLayout
{
public:
	void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& DescriptorSetLayouts, const std::vector<VkPushConstantRange>& PushConstantRanges);
	void DestroyPipelineLayout();

	VkPipelineLayout GetVkPipelineLayout() const;

private:
	VkPipelineLayout PipelineLayout;
};

class PVulkanGraphicsPipeline
{
public:
	void CreatePipeline();
	void DestroyPipeline();

	void Bind(STransform Transform);
	void Unbind();

	PVulkanPipelineLayout* GetPipelineLayout() const;
	VkPipeline GetVkPipeline() const;

protected:
	PVulkanPipelineLayout* PipelineLayout;
	PVulkanDescriptorSetLayout* DescriptorSetLayout;
	PVulkanDescriptorSet* DescriptorSet;
	PVulkanShader* VertexShader;
	PVulkanShader* FragmentShader;

public:
	SVulkanBuffer* StorageBuffer; // Array of instance data (eg. model matrix)
	SVulkanBuffer* UniformBuffer; // Global data (eg. skylight, projection and view matrix)

private:
	VkPipeline Pipeline;
};