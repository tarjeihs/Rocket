#pragma once

#include "Renderer/Common/Shader.h"
#include "Types/SharedPtr.h"

class FVkDescriptorSet;
class FVkDescriptorSetLayout;
class PVulkanShader;

struct FVkPipelineLayoutCreateInfo
{
	TArray<VkDescriptorSetLayout> DescriptorSetLayouts;
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
};

struct FVkPipelineCreateInfo
{
	TSharedPtr<FVkPipelineLayout> PipelineLayout;

	TArray<TSharedPtr<PVulkanShader>> Shaders;
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
};