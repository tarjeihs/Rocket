#pragma once

#include "Renderer/Common/Shader.h"

struct FVulkanShaderInfo
{
	VkShaderModule ShaderModule;
	VkShaderStageFlagBits Stage;
};

class PVulkanShader : public IShader
{
public:
	FVulkanShaderInfo Info;

	virtual void CreateShader(FShaderCreateInfo& CreateInfo) override;
	virtual void DestroyShader() override;
};