#pragma once

#include "Renderer/Common/Shader.h"

struct FVulkanShaderInfo
{
	VkShaderModule Module;
	VkShaderStageFlagBits Stage;
};

class FVkShader : public IShader
{
public:
	FVulkanShaderInfo Info;

	virtual void CreateShader(FShaderCreateInfo& CreateInfo) override;
	virtual void Shutdown() override;
};