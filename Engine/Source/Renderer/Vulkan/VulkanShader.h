#pragma once

#include "Renderer/Common/Shader.h"

class PVulkanDescriptorSetLayout;

class PVulkanShader : public PShader
{
public:
	void CreateShader(const std::string& ShaderSourcePath, const std::string& Entrypoint, const std::string& TargetProfile);
	void DestroyShader();

	virtual void Cleanup() override;

	VkShaderModule GetVkShaderModule() const;
	const std::vector<PVulkanDescriptorSetLayout*>& GetDescriptorSetLayouts() const;

private: 	
	VkShaderModule ShaderModule;

	std::vector<PVulkanDescriptorSetLayout*> DescriptorSetLayouts;
};