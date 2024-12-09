#pragma once

#include "Types/SharedPtr.h"

class FVkDescriptorSet;
class FVkDescriptorSetLayout;
class PVulkanShader;

struct FVulkanPipelineLayoutCreateInfo
{
	TArray<VkDescriptorSetLayout> DescriptorSetLayouts;
};

struct FVulkanPipelineLayout
{
	VkPipelineLayout Handle;
};

class PVulkanPipelineLayoutData
{
public:
	void AddPipelineLayout(const FString& Name, const FVulkanPipelineLayoutCreateInfo& CreateInfo);

	TSharedPtr<FVulkanPipelineLayout> GetPipelineLayout(const FString& Name);

private:
	TMap<FString, FVulkanPipelineLayout> PipelineStateCache; 
};




struct FVulkanPipelineStateCreateInfo
{
	TArray<TSharedPtr<PVulkanShader>> Stages;

	TSharedPtr<FVulkanPipelineLayout> PipelineLayout;
};

struct FVulkanPipelineState
{
	VkPipeline Handle;
};

class PVulkanPipelineStateData
{
public:
	void AddPipelineState(const FString& Name, const FVulkanPipelineStateCreateInfo& CreateInfo);

	TSharedPtr<FVulkanPipelineState> GetPipelineState(const FString& Name);

private:
	TMap<FString, FVulkanPipelineState> PipelineStateCache; 
};