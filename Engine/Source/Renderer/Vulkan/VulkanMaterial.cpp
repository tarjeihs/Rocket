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
    
    for (int32_t Index = 0; Index < 3; ++Index)
    {
        for (const auto& DescriptorSetLayout : VertexShader->GetDescriptorSetLayouts())
        {
            PVulkanDescriptorSet* DescriptorSet = new PVulkanDescriptorSet();
            DescriptorSet->CreateDescriptorSet(DescriptorSetLayout);
            MaterialFrameData[Index].DescriptorSets.push_back(DescriptorSet);
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
        PVulkanDescriptorSet* DescriptorSet = MaterialFrameData[FramePool->GetCurrentFrameIndex()].DescriptorSets[0];
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
    for (const auto& FrameData : MaterialFrameData)
    {
        for (const auto& DescriptorSet : FrameData.DescriptorSets)
        {
            DescriptorSet->DestroyDescriptorSet();
        }
    }
}

void PVulkanMaterial::Bind() const
{
    PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();

    std::vector<VkDescriptorSet> DescriptorSetData;
    for (const auto& DescriptorSet : MaterialFrameData[FramePool->GetCurrentFrameIndex()].DescriptorSets)
    {
        DescriptorSetData.push_back(DescriptorSet->GetVkDescriptorSet());
    }

    GraphicsPipeline->Bind(DescriptorSetData);
}

void PVulkanMaterial::Unbind() const
{
    GraphicsPipeline->Unbind();
}

void PVulkanMaterial::SetValue(const uint32_t Set, const std::string& Name, const void* Data, const size_t Size, const size_t Offset)
{
    PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();
    PVulkanDescriptorSet* DescriptorSet = MaterialFrameData[FramePool->GetCurrentFrameIndex()].DescriptorSets[Set];
    //PVulkanBuffer* Buffer = static_cast<PVulkanBuffer*>(DescriptorSet->GetBinding(Name)->Data);
    //Buffer->Submit(Data, Size, Offset);
}

void PVulkanMaterial::SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat4 Value)
{
    // TODO: Store the frame data inside descriptor set instead.
    //PVulkanDescriptorSet* DescriptorSet = DescriptorSets[Set]->GetCurrentFrame();
    
    PVulkanFramePool* FramePool = GetRHI()->GetSceneRenderer()->GetParallelFramePool();
    PVulkanDescriptorSet* DescriptorSet = MaterialFrameData[FramePool->GetCurrentFrameIndex()].DescriptorSets[Set];
    
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