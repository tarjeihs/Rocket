#pragma once

#include <vector>

class IMesh;
class PVulkanDescriptorSet;
class PVulkanDescriptorSetLayout;
class PVulkanShader;
class PVulkanCommandBuffer;
struct SBuffer;
struct SUniformBufferObject;
struct VkPipeline_T;
struct VkPipelineLayout_T;

typedef struct VkPipeline_T* VkPipeline;
typedef struct VkPipelineLayout_T* VkPipelineLayout;

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

	void Bind();
	void Unbind();

	PVulkanPipelineLayout* GetPipelineLayout() const;
	VkPipeline GetVkPipeline() const;

protected:
	PVulkanPipelineLayout* PipelineLayout;
	PVulkanShader* VertexShader;
	PVulkanShader* FragmentShader;

private:
	VkPipeline Pipeline;
};