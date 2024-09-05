#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "Math/Transform.h"

class IMesh;
class PVulkanRHI;
class PVulkanDescriptorSet;
class PVulkanDescriptorSetLayout;
class PVulkanShader;
class PVulkanCommandBuffer;
struct SBuffer;
struct SUniformBufferObject;

class PVulkanPipelineLayout
{
public:
	void CreatePipelineLayout(PVulkanRHI* RHI, const std::vector<VkDescriptorSetLayout>& DescriptorSetLayouts, const std::vector<VkPushConstantRange>& PushConstantRanges);
	void DestroyPipelineLayout(PVulkanRHI* RHI);

	VkPipelineLayout GetVkPipelineLayout() const;

private:
	VkPipelineLayout PipelineLayout;
};

class PVulkanGraphicsPipeline
{
public:
	void CreatePipeline(PVulkanRHI* RHI);
	void DestroyPipeline(PVulkanRHI* RHI);

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
	SBuffer* StorageBuffer; // Array of instance data (eg. model matrix)
	SBuffer* UniformBuffer; // Global data (eg. skylight, projection and view matrix)

private:
	VkPipeline Pipeline;
};