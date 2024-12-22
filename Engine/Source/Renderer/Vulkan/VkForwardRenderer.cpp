#include "Core/Engine.h"
#include "EnginePCH.h"
#include "VkForwardRenderer.h"

#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Renderer/Vulkan/Pipeline/VkOpaqueScriptableRendererPipeline.h"
#include "Scene/Scene.h"
#include "VulkanTexture2D.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"

void FVkForwardRenderer::Init()
{
    Super::Init();

	FVkPipelineLayout*			PipelineLayout							= new FVkPipelineLayout();

	FVkDescriptorSetLayout* 	BufferDescriptorSetLayout 				= new FVkDescriptorSetLayout();
	FVkDescriptorSetLayout* 	TextureDescriptorSetLayout 				= new FVkDescriptorSetLayout();
	FVkDescriptorSet* 			BufferDescriptorSet 					= new FVkDescriptorSet();
	FVkDescriptorSet* 			TextureDescriptorSet 					= new FVkDescriptorSet();
	FVkDescriptorPool* 			DescriptorPool 							= new FVkDescriptorPool();

	FVkBuffer** 				GlobalBuffer 							= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				CameraBuffer 							= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				MaterialBuffer 							= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				InstanceBuffer 							= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];

	FVkTexture2D*				AlbedoTexture = new FVkTexture2D();
	FVkTexture2D*				NormalTexture = new FVkTexture2D();
	//FVkTexture2D*				RoughnessTexture = new FVkTexture2D();
	//FVkTexture2D*				MetallicTexture = new FVkTexture2D();

	FVkDescriptorSetLayoutCreateInfo BufferDescriptorSetLayoutCreateInfo = 
	{{
		{ EVkDescriptorType::SSBO, 1 },
		{ EVkDescriptorType::SSBO, 1 },
		{ EVkDescriptorType::SSBO, 1 },
		{ EVkDescriptorType::SSBO, 1 },
	}};

	FVkDescriptorSetLayoutCreateInfo ImageDescriptorSetLayoutCreateInfo = 
	{{
		{ EVkDescriptorType::SamplerImage, 65536 },
		{ EVkDescriptorType::SamplerImage, 65536 },
		{ EVkDescriptorType::SamplerImage, 65536 },
		{ EVkDescriptorType::SamplerImage, 65536 },
	}};

	FVkDescriptorSetCreateInfo BufferDescriptorSetCreateInfo =
	{
		DescriptorPool,
		BufferDescriptorSetLayout
	};

	FVkDescriptorSetCreateInfo ImageDescriptorSetCreateInfo =
	{
		DescriptorPool,
		TextureDescriptorSetLayout
	};

	FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = 
	{
		{
    		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 	4.0f },
    		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 	4.0f },
		},
		2,
		VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT
	};

	FVkPipelineLayoutCreateInfo PipelineLayoutCreateInfo =
	{
		{ 
			BufferDescriptorSetLayout, 
			TextureDescriptorSetLayout 
		}
	};

	FVkBufferCreateInfo GlobalBufferCreateInfo 				= { VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 64 };
	FVkBufferCreateInfo CameraBufferCreateInfo 				= { VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 160 * 4 };
	FVkBufferCreateInfo MaterialBufferCreateInfo 			= { VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 48 * 256 };
	FVkBufferCreateInfo InstanceBufferCreateInfo 			= { VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 128 * 1024 * 1024 };

	FVkTexture2DCreateInfo AlbedoTexture2DCreateInfo 		= { "C:/Workspace/Game/Game/Content/sutr_tmave_sedy.jpeg", VK_FORMAT_R8G8B8A8_SRGB };
	FVkTexture2DCreateInfo NormalTexture2DCreateInfo 		= { "C:/Workspace/Game/Game/Content/sutr_tmave_sedy_NormalsMap.jpeg", VK_FORMAT_R8G8B8A8_UNORM };
	//FVkTexture2DCreateInfo RoughnessTexture2DCreateInfo 	= { "C:/Workspace/Game/Game/Content/download.png", VK_FORMAT_R8_UNORM };
	//FVkTexture2DCreateInfo MetallicTexture2DCreateInfo 		= { "C:/Workspace/Game/Game/Content/download.png", VK_FORMAT_R8_UNORM };

	DescriptorPool->Initialize(DescriptorPoolCreateInfo);
	BufferDescriptorSetLayout->Initialize(BufferDescriptorSetLayoutCreateInfo);
	TextureDescriptorSetLayout->Initialize(ImageDescriptorSetLayoutCreateInfo);
	BufferDescriptorSet->Initialize(BufferDescriptorSetCreateInfo);
	TextureDescriptorSet->Initialize(ImageDescriptorSetCreateInfo);
	PipelineLayout->Initialize(PipelineLayoutCreateInfo);

	for (SizeType FrameIndex = 0; FrameIndex < CONCURRENT_FRAME_COUNT; ++FrameIndex)
	{
		GlobalBuffer[FrameIndex] = new FVkBuffer();
		CameraBuffer[FrameIndex] = new FVkBuffer();
		MaterialBuffer[FrameIndex] = new FVkBuffer();
		InstanceBuffer[FrameIndex] = new FVkBuffer();

		GlobalBuffer[FrameIndex]->Initialize(GlobalBufferCreateInfo);
		CameraBuffer[FrameIndex]->Initialize(CameraBufferCreateInfo);
		MaterialBuffer[FrameIndex]->Initialize(MaterialBufferCreateInfo);
		InstanceBuffer[FrameIndex]->Initialize(InstanceBufferCreateInfo);
	}

	BufferDescriptorSet->Info.Buffers.Add(GlobalBuffer);
	BufferDescriptorSet->Info.Buffers.Add(CameraBuffer);
	BufferDescriptorSet->Info.Buffers.Add(MaterialBuffer);
	BufferDescriptorSet->Info.Buffers.Add(InstanceBuffer);

	TextureDescriptorSet->Info.Textures.Add(AlbedoTexture);
	TextureDescriptorSet->Info.Textures.Add(NormalTexture);
	//TextureDescriptorSet->Info.Textures.Add(RoughnessTexture);
	//TextureDescriptorSet->Info.Textures.Add(MetallicTexture);

	BufferDescriptorSet->WriteBuffer(0, GlobalBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]);
	BufferDescriptorSet->WriteBuffer(1, CameraBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]);
	BufferDescriptorSet->WriteBuffer(2, MaterialBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]);
	BufferDescriptorSet->WriteBuffer(3, InstanceBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]);

	AlbedoTexture->Initialize(AlbedoTexture2DCreateInfo);
	NormalTexture->Initialize(NormalTexture2DCreateInfo);
	//RoughnessTexture->Initialize(RoughnessTexture2DCreateInfo);
	//MetallicTexture->Initialize(MetallicTexture2DCreateInfo);
	
	TextureDescriptorSet->WriteTexture2D(0, AlbedoTexture);
	TextureDescriptorSet->WriteTexture2D(1, NormalTexture);
	//TextureDescriptorSet->WriteTexture2D(2, RoughnessTexture);
	//TextureDescriptorSet->WriteTexture2D(3, MetallicTexture);

	DescriptorPoolData.Add(DescriptorPool);
	DescriptorSetLayoutData.Add(BufferDescriptorSetLayout);
	DescriptorSetLayoutData.Add(TextureDescriptorSetLayout);
	DescriptorSetData.Add(BufferDescriptorSet);
	DescriptorSetData.Add(TextureDescriptorSet);

	FVkOpaqueScriptableRendererPipeline* OpaqueSRP = new FVkOpaqueScriptableRendererPipeline();
	OpaqueSRP->Initialize(PipelineLayout);
	ScriptableRendererPipelineData.Insert("Opaque", OpaqueSRP);

	SharedPipelineLayout = PipelineLayout;

	GetRenderGraph()->AddCommand([=](PVulkanCommandBuffer* CB) mutable
	{
		struct FGlobalData
		{
    		float Time;
		} GlobalData;
		GlobalData.Time = GetEngine()->Time.GetElapsedTimeAsSeconds();
		
		GlobalBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(&GlobalData, sizeof(FGlobalData));
	});

	GetRenderGraph()->AddCommand([=](PVulkanCommandBuffer* CB) mutable
	{
		struct FCameraData
		{
			alignas(16) glm::mat4 View;
			alignas(16) glm::mat4 Projection;
			alignas(16) glm::vec3 Position;
			alignas(16) glm::vec3 Direction;
		} CameraData;
		CameraData.View = GetScene()->GetCamera()->GetViewMatrix();
		CameraData.Projection = GetScene()->GetCamera()->GetProjectionMatrix();
		CameraData.Position = GetScene()->GetCamera()->GetPosition();
		CameraData.Direction = GetScene()->GetCamera()->GetRotation();

		CameraBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(&CameraData, sizeof(FCameraData));
	});

	GetRenderGraph()->AddCommand([=](PVulkanCommandBuffer* CB) mutable
	{
		struct FMaterialData
		{
			alignas(4) uint32 AlbedoTextureID;
    		alignas(4) uint32 NormalTextureID;
		} MaterialData;
		MaterialData.AlbedoTextureID = 0;
		MaterialData.NormalTextureID = 0;

		MaterialBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(&MaterialData, sizeof(FMaterialData));
	});

	GetRenderGraph()->AddCommand([=](PVulkanCommandBuffer* CB) mutable
	{
		struct FInstanceData
		{
			alignas(16) glm::mat4 Transform;
			alignas(16) glm::mat4 TransformInverseTranspose;
		} InstanceData;
		InstanceData.Transform = glm::identity<glm::mat4>();
		InstanceData.TransformInverseTranspose = glm::inverseTranspose(glm::identity<glm::mat4>());

		InstanceBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(&InstanceData, sizeof(FInstanceData));
	});
}

void FVkForwardRenderer::Shutdown() 
{
    Super::Shutdown();

	vkDestroyPipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), SharedPipelineLayout->Info.Handle, VK_NULL_HANDLE);

	for (const auto& DescriptorSet : DescriptorSetData)
	{
		DescriptorSet->Shutdown();
	}

	for (const auto& DescriptorSetLayout : DescriptorSetLayoutData)
	{
		DescriptorSetLayout->Destroy();
	}

	for (const auto& DescriptorPool : DescriptorPoolData)
	{
		DescriptorPool->Shutdown();
	}

	for (const auto& Pair : ScriptableRendererPipelineData)
	{
		Pair.Value->Shutdown();
	}
}

void FVkForwardRenderer::Bind()
{
	PROFILE_FUNC_SCOPE("FVkForwardRenderer::Bind")

	TArray<VkDescriptorSet> DescriptorSets;
	for (const auto& DescriptorSet : DescriptorSetData)
	{
		DescriptorSets.Add(DescriptorSet->Info.Handle);
	}
	vkCmdBindDescriptorSets(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, SharedPipelineLayout->Info.Handle, 0, DescriptorSets.GetSize(), DescriptorSets.GetData(), 0, 0);

	for (const auto& Pair : ScriptableRendererPipelineData)
	{
		Pair.Value->Bind();
	}
}