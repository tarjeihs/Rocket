#pragma once

#include <vulkan/vulkan_core.h>

enum EShaderType : uint8_t
{
	RK_SHADERTYPE_NONE = 0,
	RK_SHADERTYPE_VERTEXSHADER,		// vs_6_0
	RK_SHADERTYPE_FRAGMENTSHADER,	// fs_6_0
	RK_SHADERTYPE_COMPUTE_SHADER	// cs_6_0
};

struct SShaderProgram
{
	VkPipelineShaderStageCreateInfo CreateInfo;

	// Compiled SPIR-V bytecode. Note: Should be freed right after compilation as Vulkan creates an internal copy.
	VkShaderModule ShaderModule;
};

class PVulkanShader
{
public:
	SShaderProgram Compile(VkDevice LogicalDevice, EShaderType ShaderType, const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile);
	
	void Free(VkDevice LogicalDevice, SShaderProgram& ShaderProgram);

private:
	std::vector<SShaderProgram> ShaderPrograms;
};