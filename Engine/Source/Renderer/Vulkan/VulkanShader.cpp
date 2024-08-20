#include "VulkanShader.h"

#include "Core/Assert.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanDevice.h"

void PVulkanShader::CreateShader(PVulkanRHI* RHI, const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile)
{
	CompileShaderHLSL(ShaderSourcePath, Entrypoint, TargetProfile, [&](const ComPtr<IDxcBlob>& ShaderBlob)
	{
		VkShaderModuleCreateInfo ShaderModuleCreateInfo{};
		ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ShaderModuleCreateInfo.codeSize = ShaderBlob->GetBufferSize();
		ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(ShaderBlob->GetBufferPointer());

		VkResult Result = vkCreateShaderModule(RHI->GetDevice()->GetVkDevice(), &ShaderModuleCreateInfo, nullptr, &ShaderModule);
		RK_ASSERT(Result == VK_SUCCESS, "Failed to create Shader Module.");
	});
}

void PVulkanShader::DestroyShader(PVulkanRHI* RHI)
{
	vkDestroyShaderModule(RHI->GetDevice()->GetVkDevice(), ShaderModule, nullptr);
}

VkShaderModule PVulkanShader::GetVkShaderModule() const
{
	return ShaderModule;
}
