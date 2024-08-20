#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

class PVulkanRHI;
class PVulkanDescriptorSet;
class PVulkanDescriptorSetLayout;
class PVulkanShader;
class PVulkanCommandBuffer;
class PVulkanMesh;
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
	void BindPipeline(PVulkanRHI* RHI, PVulkanCommandBuffer* CommandBuffer, PVulkanMesh* Mesh, const SUniformBufferObject& GlobalUBO);

	VkPipeline GetVkPipeline() const;

protected:
	PVulkanPipelineLayout* PipelineLayout;
	
	PVulkanDescriptorSetLayout* DescriptorSetLayout;
	PVulkanDescriptorSet* DescriptorSet;

	PVulkanShader* VertexShader;
	PVulkanShader* FragmentShader;

	SBuffer* UniformBuffer;

private:
	VkPipeline Pipeline;
};