#pragma once

class FVkDescriptorSet;
class FVkDescriptorSetLayout;
class FVkShader;
class PVulkanImage;
class FVkBuffer;

struct FVkPipelineLayoutCreateInfo
{
	TArray<FVkDescriptorSetLayout*> DescriptorSetLayouts;
};

struct FVkPipelineLayoutInfo
{
	VkPipelineLayout Handle;
};

class FVkPipelineLayout
{
public:
	FVkPipelineLayoutInfo Info;

	void Initialize(const FVkPipelineLayoutCreateInfo& CreateInfo);
	void Shutdown();
};

struct FVkPipelineCreateInfo
{
	TArray<FVkShader*> Shaders;

	FVkPipelineLayout* PipelineLayout;
};

struct FVkPipelineInfo
{
	VkPipeline Handle;
};

class FVkPipeline
{
public:
	FVkPipelineInfo Info;

	void Initialize(FVkPipelineCreateInfo& CreateInfo);
	void Shutdown();
};