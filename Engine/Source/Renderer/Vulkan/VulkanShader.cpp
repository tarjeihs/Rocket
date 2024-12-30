#include "EnginePCH.h"
#include "VulkanShader.h"

#include "Format/HLSL.h"
#include "Renderer/Common/Shader.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanPipeline.h"

namespace Utils
{
	VkShaderStageFlagBits GetVkShaderStage(const EShaderStage ShaderStage)
	{
		switch (ShaderStage)
		{
			case EShaderStage::Vertex: 		return VK_SHADER_STAGE_VERTEX_BIT;
			case EShaderStage::Pixel: 		return VK_SHADER_STAGE_FRAGMENT_BIT;
			case EShaderStage::Compute: 	return VK_SHADER_STAGE_COMPUTE_BIT;
		}
		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}

	std::string GetTextShaderStage(const EShaderStage ShaderStage)
	{
		switch (ShaderStage)
		{
			case EShaderStage::Vertex: 		return "vs_6_0";
			case EShaderStage::Pixel: 		return "ps_6_0";
			case EShaderStage::Compute: 	return "cs_6_0";
		}
		return std::string();
	}
}

void FVkShader::Init(FShaderCreateInfo& CreateInfo)
{
    FHLSL HLSL = Format::ImportHLSL(CreateInfo.Path, "main", Utils::GetTextShaderStage(CreateInfo.Stage));

	VkShaderModuleCreateInfo ShaderModuleCreateInfo{};
	ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderModuleCreateInfo.codeSize = HLSL.Size;
	ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(HLSL.Data);

	VkResult Result = vkCreateShaderModule(GetRHI()->GetDevice()->GetVkDevice(), &ShaderModuleCreateInfo, nullptr, &Info.Module);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create Shader Module.");

	Info.Stage = Utils::GetVkShaderStage(CreateInfo.Stage);
}

void FVkShader::Free()
{
	vkDestroyShaderModule(GetRHI()->GetDevice()->GetVkDevice(), Info.Module, nullptr);
}