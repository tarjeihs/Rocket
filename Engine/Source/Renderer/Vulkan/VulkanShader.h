#pragma once

#include "Renderer/Common/Shader.h"

class PVulkanDescriptorSetLayout;

struct SShaderModule
{
	VkShaderModule ShaderModule;
	VkShaderStageFlagBits Flag;

	std::vector<PVulkanDescriptorSetLayout*> DescriptorSetLayouts;
};

class PVulkanShader : public IShader
{
public:
	virtual void CreateShader(std::vector<SShaderModuleBinary> ShaderBinaryObject) override;
	virtual void DestroyShader() override;

	std::span<SShaderModule> GetShaderModules();

private: 	
	std::vector<SShaderModule> ShaderModules;
};