#pragma once

#include <vector>

class IMesh;
class PVulkanDescriptorSet;
class PVulkanDescriptorSetLayout;
class PVulkanShader;

class PVulkanPipelineLayout
{
public:
	void CreatePipelineLayout(const std::vector<PVulkanDescriptorSetLayout*> DescriptorSetLayouts, const std::vector<VkPushConstantRange>& PushConstantRanges);
	void DestroyPipelineLayout();

	VkPipelineLayout GetVkPipelineLayout() const;

private:
	VkPipelineLayout PipelineLayout;
};

class PVulkanGraphicsPipeline
{
public:
	void CreatePipeline(PVulkanShader* Shader);
	void DestroyPipeline();

	void Bind(std::vector<VkDescriptorSet> Data);
	void Unbind();

	PVulkanPipelineLayout* GetPipelineLayout() const;
	VkPipeline GetVkPipeline() const;

protected:
	PVulkanPipelineLayout* PipelineLayout;

private:
	VkPipeline Pipeline;
};