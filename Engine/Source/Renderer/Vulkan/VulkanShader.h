#pragma once

#include "Renderer/Common/Shader.h"

class PVulkanRHI;
struct VkShaderModule_T;

typedef VkShaderModule_T* VkShaderModule;

class PVulkanShader : public PShader
{
public:
	void CreateShader(const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile);
	void DestroyShader();

	VkShaderModule GetVkShaderModule() const;

private:
	VkShaderModule ShaderModule;
};