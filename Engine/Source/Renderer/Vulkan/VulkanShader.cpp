#include "EnginePCH.h"
#include "VulkanShader.h"

#include <spirv_cross/spirv.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_cross_containers.hpp>

#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"

void PVulkanShader::CreateShader(std::vector<SShaderModuleBinary> ShaderBinaryObject)
{
#ifdef RK_PLATFORM_LINUX
	for (int32_t Index = 0; Index < ShaderBinaryObject.size(); ++Index)
	{
		SShaderModule ShaderModule;

		VkShaderModuleCreateInfo ShaderModuleCreateInfo{};
		ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ShaderModuleCreateInfo.codeSize = ShaderBinaryObject[Index].Size;

		ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(ShaderBinaryObject[Index].Data);

		VkResult Result = vkCreateShaderModule(GetRHI()->GetDevice()->GetVkDevice(), &ShaderModuleCreateInfo, nullptr, &ShaderModule.ShaderModule);
		RK_ASSERT(Result == VK_SUCCESS, "Failed to create Shader Module.");

		std::vector<uint32_t> Binary(reinterpret_cast<const uint32_t*>(ShaderBinaryObject[Index].Data), reinterpret_cast<const uint32_t*>(ShaderBinaryObject[Index].Data) + (ShaderBinaryObject[Index].Size / sizeof(uint32_t)));
		spirv_cross::Compiler Compiler(Binary);
		spirv_cross::ShaderResources Resources = Compiler.get_shader_resources();
		spirv_cross::EntryPoint Entrypoint = Compiler.get_entry_points_and_stages()[0];
		spv::ExecutionModel ExecutionModel = Entrypoint.execution_model;

		switch (ExecutionModel)
		{
			case spv::ExecutionModelVertex: ShaderModule.Flag = VK_SHADER_STAGE_VERTEX_BIT; break;
			case spv::ExecutionModelFragment: ShaderModule.Flag = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			default: break;
		}

		std::unordered_map<uint32_t, std::vector<SDescriptorSetBindingLayout>> Sets;
		for (const auto& Resource : Resources.uniform_buffers)
		{
			uint32_t Set = Compiler.get_decoration(Resource.id, spv::DecorationDescriptorSet);
			uint32_t Binding = Compiler.get_decoration(Resource.id, spv::DecorationBinding);

			spirv_cross::SPIRType Type = Compiler.get_type(Resource.base_type_id);
			size_t BufferStride = Compiler.get_declared_struct_size(Type);

			SDescriptorSetBindingLayout DescriptorSetBinding;
			DescriptorSetBinding.Name = Compiler.get_name(Resource.id);
			DescriptorSetBinding.Binding = Binding;
			DescriptorSetBinding.Flag = EDescriptorSetBindingFlag::Vertex;
			DescriptorSetBinding.Type = EDescriptorSetBindingType::Uniform;
			DescriptorSetBinding.Size = BufferStride;

			for (uint32_t Index = 0; Index < Type.member_types.size(); ++Index)
			{
				spirv_cross::SPIRType MemberType = Compiler.get_type(Type.member_types[Index]);

				std::string MemberName = Compiler.get_member_name(Resource.base_type_id, Index);
				size_t MemberSize = Compiler.get_declared_struct_member_size(Type, Index); 
				size_t MemberOffset = Compiler.type_struct_member_offset(Type, Index); 

				SDescriptorSetBindingMemberLayout DescriptorSetBindingMember;
				DescriptorSetBindingMember.Name = MemberName;
				DescriptorSetBindingMember.Size = MemberSize;
				DescriptorSetBindingMember.Offset = MemberOffset;

				DescriptorSetBinding.Members.push_back(DescriptorSetBindingMember);
			}

			Sets[Set].push_back(DescriptorSetBinding);
		}

		for (const auto& Resource : Resources.storage_buffers)
		{
			uint32_t Set = Compiler.get_decoration(Resource.id, spv::DecorationDescriptorSet);
			uint32_t Binding = Compiler.get_decoration(Resource.id, spv::DecorationBinding);

			spirv_cross::SPIRType Type = Compiler.get_type(Resource.base_type_id);
			size_t BufferStride = Compiler.type_struct_member_array_stride(Type, Type.member_types.size() - 1);

			SDescriptorSetBindingLayout DescriptorSetBinding;
			DescriptorSetBinding.Name = Compiler.get_name(Resource.id);
			DescriptorSetBinding.Binding = Binding;
			DescriptorSetBinding.Size = BufferStride * 1024 * 10;
			DescriptorSetBinding.Flag = EDescriptorSetBindingFlag::Vertex;
			DescriptorSetBinding.Type = EDescriptorSetBindingType::Storage;

			Sets[Set].push_back(DescriptorSetBinding);
		}

		for (const auto& PushConstant : Resources.push_constant_buffers)
		{
			//VkPushConstantRange pushConstantRange = {};
			//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;  // Modify based on shader stage usage
			//pushConstantRange.offset = 0;  // Push constants start at offset 0
			//pushConstantRange.size = static_cast<uint32_t>(Compiler.get_declared_struct_size(Compiler.get_type(PushConstant.base_type_id))); // Size of push constant block
			//PushConstantRanges.push_back(pushConstantRange);
		}

		for (const auto& Set : Sets)
		{
			const std::vector<SDescriptorSetBindingLayout>& Bindings = Set.second;

			PVulkanDescriptorSetLayout* DescriptorSetLayout = new PVulkanDescriptorSetLayout();
			DescriptorSetLayout->CreateDescriptorSetLayout(Bindings);

			ShaderModule.DescriptorSetLayouts.push_back(DescriptorSetLayout);
		}

		free(ShaderBinaryObject[Index].Data);
		ShaderBinaryObject[Index].Data = nullptr;
		ShaderBinaryObject[Index].Size = 0;

		ShaderModules.push_back(ShaderModule);
	}
#endif
}

void PVulkanShader::DestroyShader()
{
	for (int32_t Index = 0; Index < ShaderModules.size(); ++Index)
	{
		vkDestroyShaderModule(GetRHI()->GetDevice()->GetVkDevice(), ShaderModules[Index].ShaderModule, nullptr);

		for (PVulkanDescriptorSetLayout* Layout : ShaderModules[Index].DescriptorSetLayouts)
		{
			Layout->DestroyDescriptorSetLayout();
		}
	}
}

std::span<SShaderModule> PVulkanShader::GetShaderModules()
{
	return ShaderModules;
}