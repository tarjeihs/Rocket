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
			case EShaderStage::Fragment: 	return VK_SHADER_STAGE_FRAGMENT_BIT;
			case EShaderStage::Compute: 	return VK_SHADER_STAGE_COMPUTE_BIT;
		}
	}
}

void PVulkanShader::CreateShader(FShaderCreateInfo& CreateInfo)
{
    FHLSL HLSL = Format::ImportHLSL(CreateInfo.Path, "main", "vs_6_0");

	VkShaderModuleCreateInfo ShaderModuleCreateInfo{};
	ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderModuleCreateInfo.codeSize = HLSL.Size;
	ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(HLSL.Data);

	VkResult Result = vkCreateShaderModule(GetRHI()->GetDevice()->GetVkDevice(), &ShaderModuleCreateInfo, nullptr, &Info.Module);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create Shader Module.");

	Info.Stage = Utils::GetVkShaderStage(CreateInfo.Stage);

	//TArray<FVulkanDescriptorPoolRatio> StorageImagePoolRatio = {
//
	//	{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 65536 },
	//};
//
	//TArray<FVulkanDescriptorPoolRatio> StorageBufferPoolRatio = {
//
	//	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 65536 },
	//};
//
	//FVulkanDescriptorPoolCreateInfo StorageImageDescriptorPoolCreateInfo;
	//StorageImageDescriptorPoolCreateInfo.PoolRatios = StorageImagePoolRatio;
	//StorageImageDescriptorPoolCreateInfo.MaxSetCount = FRAMES_IN_FLIGHT;
	//StorageImageDescriptorPoolCreateInfo.Flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
//
	//FVulkanDescriptorPoolCreateInfo StorageBufferDescriptorPoolCreateInfo;
	//StorageBufferDescriptorPoolCreateInfo.PoolRatios = StorageBufferPoolRatio;
	//StorageBufferDescriptorPoolCreateInfo.MaxSetCount = FRAMES_IN_FLIGHT;
	//StorageBufferDescriptorPoolCreateInfo.Flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
//
	//TSharedPtr<PVulkanDescriptorPool> StorageImageDescriptorPool = MakeShared<PVulkanDescriptorPool>();
	//StorageImageDescriptorPool->CreatePool(StorageImageDescriptorPoolCreateInfo);
//
	//TSharedPtr<PVulkanDescriptorPool> StorageBufferDescriptorPool = MakeShared<PVulkanDescriptorPool>();
	//StorageImageDescriptorPool->CreatePool(StorageBufferDescriptorPoolCreateInfo);
//
//
	//// Texture
	//FVulkanDescriptorSetLayoutCreateInfo StaticDescriptorSetLayoutCreateInfo;
	//StaticDescriptorSetLayoutCreateInfo.Descriptors = {
	//	{ EDescriptorType::Image, 65536 }
	//};
//
	//// Data
	//FVulkanDescriptorSetLayoutCreateInfo DynamicDescriptorSetLayoutCreateInfo;
	//DynamicDescriptorSetLayoutCreateInfo.Descriptors = {
	//	{ EDescriptorType::Storage, 1 }, 	// Environment
	//	{ EDescriptorType::Storage, 1 }, 	// Camera Data
	//	{ EDescriptorType::Storage, 1 }, 	// Material Data
	//	{ EDescriptorType::Storage, 1 } 	// Object Data
	//};
	//
	//TSharedPtr<PVulkanDescriptorSetLayout> StorageImageDescriptorSetLayout = MakeShared<PVulkanDescriptorSetLayout>();
	//StorageImageDescriptorSetLayout->CreateDescriptorSetLayout(StaticDescriptorSetLayoutCreateInfo);
//
	//TSharedPtr<PVulkanDescriptorSetLayout> StorageBufferDescriptorSetLayout = MakeShared<PVulkanDescriptorSetLayout>();
	//StorageBufferDescriptorSetLayout->CreateDescriptorSetLayout(DynamicDescriptorSetLayoutCreateInfo);
//
//
//
	//FVulkanGraphicsPipelineLayoutCreateInfo PipelineLayoutCreateInfo;
	//PipelineLayoutCreateInfo.DescriptorSetLayouts.Add(StorageImageDescriptorSetLayout->Info.DescriptorSetLayout);
	//PipelineLayoutCreateInfo.DescriptorSetLayouts.Add(StorageBufferDescriptorSetLayout->Info.DescriptorSetLayout);
//
	//TSharedPtr<PVulkanPipelineLayout> PipelineLayout = MakeShared<PVulkanPipelineLayout>();
	//PipelineLayout->CreatePipelineLayout(PipelineLayoutCreateInfo);
//
	//FVulkanGraphicsPipelineCreateInfo PSOCreateInfo;
	//PSOCreateInfo.PipelineLayout = PipelineLayout;
	//PSOCreateInfo.ShaderStages.Add(this);
//
	//TSharedPtr<PVulkanGraphicsPipeline> Pipeline = MakeShared<PVulkanGraphicsPipeline>();
	//Pipeline->CreatePipeline(PSOCreateInfo);
//
//
//
//
	//FVulkanDescriptorSetCreateInfo StorageImageDescriptorSetCreateInfo;
	//StorageImageDescriptorSetCreateInfo.DescriptorSetLayout = StorageImageDescriptorSetLayout;
	//StorageImageDescriptorSetCreateInfo.DescriptorPool = StorageImageDescriptorPool;
//
	//FVulkanDescriptorSetCreateInfo StorageBufferDescriptorSetCreateInfo;
	//StorageBufferDescriptorSetCreateInfo.DescriptorSetLayout = StorageBufferDescriptorSetLayout;
	//StorageBufferDescriptorSetCreateInfo.DescriptorPool = StorageBufferDescriptorPool;
//
	//TSharedPtr<PVulkanDescriptorSet> DescriptorSet = MakeShared<PVulkanDescriptorSet>();
	//DescriptorSet->CreateDescriptorSet(StorageImageDescriptorSetCreateInfo);
}

void PVulkanShader::DestroyShader()
{
	vkDestroyShaderModule(GetRHI()->GetDevice()->GetVkDevice(), Info.Module, nullptr);
}