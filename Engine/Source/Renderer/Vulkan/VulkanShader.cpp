#include "EnginePCH.h"
#include "VulkanShader.h"

#include "Renderer/Vulkan/VulkanDevice.h"

void PVulkanShader::CreateShader(const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile)
{
#ifdef RK_PLATFORM_WINDOWS
	CompileShaderHLSL(ShaderSourcePath, Entrypoint, TargetProfile, [&](const ComPtr<IDxcBlob>& ShaderBlob)
	{
		VkShaderModuleCreateInfo ShaderModuleCreateInfo{};
		ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ShaderModuleCreateInfo.codeSize = ShaderBlob->GetBufferSize();
		ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(ShaderBlob->GetBufferPointer());

		VkResult Result = vkCreateShaderModule(GetRHI()->GetDevice()->GetVkDevice(), &ShaderModuleCreateInfo, nullptr, &ShaderModule);
		RK_ASSERT(Result == VK_SUCCESS, "Failed to create Shader Module.");
	});
#endif

#ifdef RK_PLATFORM_LINUX
	CompileShaderHLSL(ShaderSourcePath, Entrypoint, TargetProfile, [&](const CComPtr<IDxcBlob>& ShaderBlob)
	{
		VkShaderModuleCreateInfo ShaderModuleCreateInfo{};
		ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ShaderModuleCreateInfo.codeSize = ShaderBlob->GetBufferSize();
		ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(ShaderBlob->GetBufferPointer());

		VkResult Result = vkCreateShaderModule(GetRHI()->GetDevice()->GetVkDevice(), &ShaderModuleCreateInfo, nullptr, &ShaderModule);
		RK_ASSERT(Result == VK_SUCCESS, "Failed to create Shader Module.");
	});
#endif
}

void PVulkanShader::DestroyShader()
{
	vkDestroyShaderModule(GetRHI()->GetDevice()->GetVkDevice(), ShaderModule, nullptr);
}

VkShaderModule PVulkanShader::GetVkShaderModule() const
{
	return ShaderModule;
}
