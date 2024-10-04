#include "EnginePCH.h"
#include "VulkanMaterial.h"

#include "Renderer/Common/Material.h"
#include "Renderer/Common/Shader.h"
#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanMaterial::CreateMaterial(const SMaterialBinaryData& MaterialData)
{
    
}

void PVulkanMaterial::Destroy()
{
    GraphicsPipeline->DestroyPipeline();

    PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();
    for (const auto& FrameData : *FramePool)
    {
        for (const auto& DescriptorSet : FrameData->GetMemory()->DescriptorSets)
        {
            DescriptorSet->DestroyDescriptorSet();
        }
    }
}

void PVulkanMaterial::Bind() const
{
    PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();

    std::vector<VkDescriptorSet> DescriptorSetData;
    for (const auto& DescriptorSet : FramePool->GetCurrentFrame()->GetMemory()->DescriptorSets)
    {
        DescriptorSetData.push_back(DescriptorSet->GetVkDescriptorSet());
    }

    GraphicsPipeline->Bind(DescriptorSetData);
}

void PVulkanMaterial::Unbind() const
{
    GraphicsPipeline->Unbind();
}

void PVulkanMaterial::SetShader(IShader* Shader)
{
    for (const auto& Frame : *GetRHI()->GetSceneRenderer()->GetParallelFramePool())
    {
        PVulkanShader* VShader = Cast<PVulkanShader>(Shader);
        for (const SShaderModule& ShaderModule : VShader->GetShaderModules())
        {
            for (const auto& DescriptorSetLayout : ShaderModule.DescriptorSetLayouts)
            {
                PVulkanDescriptorSet* DescriptorSet = new PVulkanDescriptorSet();
                DescriptorSet->CreateDescriptorSet(DescriptorSetLayout, Frame);
                Frame->GetMemory()->DescriptorSets.push_back(DescriptorSet);
            }
        }
    }

    GraphicsPipeline = new PVulkanGraphicsPipeline();
    GraphicsPipeline->CreatePipeline(Cast<PVulkanShader>(Shader));

    GetRHI()->GetSceneRenderer()->GetRenderGraph()->AddCommand([&](PVulkanFrame* Frame)
	{
		PCamera* Camera = GetScene()->GetCamera();
    	
		SUniformBufferObject UBO;
    	UBO.ViewMatrix = Camera->GetViewMatrix();
    	UBO.ProjectionMatrix = Camera->GetProjectionMatrix();
		
        SetUniformValue(1, "UBO", "m_ViewMatrix", Camera->GetViewMatrix());
        SetUniformValue(1, "UBO", "m_ProjectionMatrix", Camera->GetProjectionMatrix());
        SetUniformValue(1, "UBO", "CameraWorldPosition", Camera->GetPosition());
	});
    
    GetRHI()->GetSceneRenderer()->GetRenderGraph()->AddCommand([this](PVulkanFrame* Frame)
	{
		std::vector<SShaderStorageBufferObject> SSBOs;

		GetScene()->GetRegistry()->View<STransformComponent, SMeshComponent>([&](const STransformComponent& TransformComponent, const SMeshComponent& MeshComponent)
		{
            PVulkanMesh* Mesh = static_cast<PVulkanMesh*>(MeshComponent.Mesh);
            if (MeshComponent.Mesh->GetMaterial() == this)
            {
                glm::mat4 ModelMatrix = TransformComponent.Transform.ToMatrix();
                glm::mat4 NormalMatrix = glm::transpose(glm::inverse(ModelMatrix));

                SShaderStorageBufferObject SSBO;
                SSBO.ModelMatrix = ModelMatrix;
                SSBO.NormalMatrix = NormalMatrix;

                SSBOs.push_back( SSBO );
            }
		});
		
        PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();
        PVulkanDescriptorSet* DescriptorSet = FramePool->GetCurrentFrame()->GetMemory()->DescriptorSets[0];
        for (const auto& Binding : DescriptorSet->GetBindings())
        {
            if (Binding.Layout->Name == "SSBO")
            {
                PVulkanBuffer* Buffer = static_cast<PVulkanBuffer*>(Binding.Data);
                Buffer->Submit(SSBOs.data(), sizeof(SShaderStorageBufferObject) * SSBOs.size());
            }
        }

		uint32_t ObjectID = 0;
    	GetScene()->GetRegistry()->View<STransformComponent, SMeshComponent>([&](const STransformComponent& TransformComponent, const SMeshComponent& MeshComponent)
    	{
            if (MeshComponent.Mesh->GetMaterial() == this)
            {
                MeshComponent.Mesh->DrawIndirectInstanced(ObjectID);
                ObjectID++;
            }
    	});
	});
}

void PVulkanMaterial::SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat4 Value)
{
    PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();
    PVulkanDescriptorSet* DescriptorSet = FramePool->GetCurrentFrame()->GetMemory()->DescriptorSets[Set];
    
    for (const SDescriptorSetBinding& Binding : DescriptorSet->GetBindings())
    {
        if (Binding.Layout->Name == UniformName)
        {
            for (auto& Member : Binding.Layout->Members)
            {
                if (Member.Name == MemberName)
                {
                    PVulkanBuffer* Buffer = static_cast<PVulkanBuffer*>(Binding.Data);
                    Buffer->Submit(&Value, Member.Size, Member.Offset);
                }
            }        
        }
    }
}

void PVulkanMaterial::SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::vec2 Value)
{

}

void PVulkanMaterial::SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::vec3 Value)
{
    PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();
    PVulkanDescriptorSet* DescriptorSet = FramePool->GetCurrentFrame()->GetMemory()->DescriptorSets[Set];
    
    for (PVulkanFrame* Frame : *FramePool)
    {

    for (const SDescriptorSetBinding& Binding : Frame->GetMemory()->DescriptorSets[Set]->GetBindings())
    {
        if (Binding.Layout->Name == UniformName)
        {
            for (auto& Member : Binding.Layout->Members)
            {
                if (Member.Name == MemberName)
                {
                    PVulkanBuffer* Buffer = static_cast<PVulkanBuffer*>(Binding.Data);
                    Buffer->Submit(&Value, Member.Size, Member.Offset);
                }
            }
        }
    }
    }

}

void PVulkanMaterial::SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, float Value)
{
    PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();
    PVulkanDescriptorSet* DescriptorSet = FramePool->GetCurrentFrame()->GetMemory()->DescriptorSets[Set];
    
    for (PVulkanFrame* Frame : *FramePool)
    {

    for (const SDescriptorSetBinding& Binding : Frame->GetMemory()->DescriptorSets[Set]->GetBindings())
    {
        if (Binding.Layout->Name == UniformName)
        {
            for (auto& Member : Binding.Layout->Members)
            {
                if (Member.Name == MemberName)
                {
                    PVulkanBuffer* Buffer = static_cast<PVulkanBuffer*>(Binding.Data);
                    Buffer->Submit(&Value, Member.Size, Member.Offset);
                }
            }
        }
    }
    }
}

void PVulkanMaterial::SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::vec4 Value)
{
    PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();
    PVulkanDescriptorSet* DescriptorSet = FramePool->GetCurrentFrame()->GetMemory()->DescriptorSets[Set];
    
    for (PVulkanFrame* Frame : *FramePool)
    {

    for (const SDescriptorSetBinding& Binding : Frame->GetMemory()->DescriptorSets[Set]->GetBindings())
    {
        if (Binding.Layout->Name == UniformName)
        {
            for (auto& Member : Binding.Layout->Members)
            {
                if (Member.Name == MemberName)
                {
                    PVulkanBuffer* Buffer = static_cast<PVulkanBuffer*>(Binding.Data);
                    Buffer->Submit(&Value, Member.Size, Member.Offset);
                }
            }
        }
    }
    }
}

void PVulkanMaterial::SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat2 Value)
{

}

void PVulkanMaterial::SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat3 Value)
{

}