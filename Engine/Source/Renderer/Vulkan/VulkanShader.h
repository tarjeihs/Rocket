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

	virtual void Init(FShaderCreateInfo& CreateInfo) override;
	virtual void Free() override;
};