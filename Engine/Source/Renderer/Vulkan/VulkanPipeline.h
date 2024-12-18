#pragma once

#include "Renderer/Settings.h"

class FVkDescriptorSet;
class FVkDescriptorSetLayout;
class FVkShader;
class FVkImage;
class FVkBuffer;

struct FVkPipelineLayoutCreateInfo
{
	TArray<VkDescriptorSetLayout> DescriptorSetLayouts;
};

struct FVkPipelineLayoutInfo
{
	VkPipelineLayout Handle;

	TArray<FVkDescriptorSetLayout*> DescriptorSetLayout;
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
	FVkPipelineLayout* PipelineLayout;
	
	TArray<FVkShader*> Shaders;
};

struct FVkPipelineInfo
{
	VkPipeline 									Handle;
	
	TArray<FVkDescriptorSet*> 					DescriptorSet					[CONCURRENT_FRAME_COUNT];
};

class FVkPipeline
{
public:
	FVkPipelineInfo Info;

	void Initialize(FVkPipelineCreateInfo& CreateInfo);
	void Shutdown();
};