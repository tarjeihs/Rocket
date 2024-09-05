#pragma once

#include <vulkan/vulkan_core.h>

#include "Renderer/Common/Shader.h"

class PVulkanRHI;

class PVulkanShader : public PShader
{
public:
	void CreateShader(const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile);
	void DestroyShader();

	VkShaderModule GetVkShaderModule() const;

private:
	VkShaderModule ShaderModule;
};