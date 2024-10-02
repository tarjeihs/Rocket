#include "EnginePCH.h"
#include "VulkanMaterial.h"

#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanMaterial::Init(PShader* BaseVertexShader, PShader* BaseFragmentShader)
{
    PVulkanShader* VertexShader = static_cast<PVulkanShader*>(BaseVertexShader);
    PVulkanShader* FragmentShader = static_cast<PVulkanShader*>(BaseFragmentShader);
    
    for (const auto& Frame : *GetRHI()->GetSceneRenderer()->GetParallelFramePool())
    {
        for (const auto& DescriptorSetLayout : VertexShader->GetDescriptorSetLayouts())
        {
            PVulkanDescriptorSet* DescriptorSet = new PVulkanDescriptorSet();
            DescriptorSet->CreateDescriptorSet(DescriptorSetLayout, Frame);
            Frame->GetMemory()->DescriptorSets.push_back(DescriptorSet);
        }
    }

    GraphicsPipeline = new PVulkanGraphicsPipeline();
    GraphicsPipeline->CreatePipeline(VertexShader->GetDescriptorSetLayouts(), VertexShader, FragmentShader);

    GetRHI()->GetSceneRenderer()->GetRenderGraph()->AddCommand([&](PVulkanFrame* Frame)
	{
		PCamera* Camera = GetScene()->GetCamera();
    	
		SUniformBufferObject UBO;
    	UBO.ViewMatrix = Camera->GetViewMatrix();
    	UBO.ProjectionMatrix = Camera->GetProjectionMatrix();
		
        SetUniformValue(1, "UBO", "m_ViewMatrix", Camera->GetViewMatrix());
        SetUniformValue(1, "UBO", "m_ProjectionMatrix", Camera->GetProjectionMatrix());
	});
    
    GetRHI()->GetSceneRenderer()->GetRenderGraph()->AddCommand([this](PVulkanFrame* Frame)
	{
		std::vector<SShaderStorageBufferObject> SSBOs;

		GetScene()->GetRegistry()->View<STransformComponent, SMeshComponent>([&](const STransformComponent& TransformComponent, const SMeshComponent& MeshComponent)
		{
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