#include "EnginePCH.h"
#include "VkForwardRenderer.h"

#include "Pipeline/VkOverlayScriptableRendererPipeline.h"
#include "Pipeline/VkPostProcessScriptableRendererPipeline.h"
#include "Renderer/RHI.h"
#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Renderer/Vulkan/Pipeline/VkOpaqueScriptableRendererPipeline.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Scene/Component.h"
#include "Scene/Scene.h"
#include "Utils/Profiler.h"
#include "VulkanTexture2D.h"
#include "VulkanImage.h"
#include "glm/gtc/matrix_inverse.hpp"

// TODO IMPORTANT: Resizing must apply to the tone mapping images aswell..!

void FVkForwardRenderer::Init()
{
    Super::Init();

	PipelineLayout												= new FVkPipelineLayout();
					
	DescriptorSet 												= new FVkDescriptorSet();
	DescriptorSetLayout 										= new FVkDescriptorSetLayout();
	DescriptorPool 												= new FVkDescriptorPool();

	FVkBuffer** 				GlobalBuffer 					= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				CameraBuffer 					= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				MaterialBuffer 					= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				InstanceBuffer 					= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	
	FVkImage**					HDRImage 						= new FVkImage*				[CONCURRENT_FRAME_COUNT];
	FVkImage**					SDRImage 						= new FVkImage*				[CONCURRENT_FRAME_COUNT];

	FVkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo =
	{{
		{ EVkDescriptorType::StructuredBuffer, 	16   },
		{ EVkDescriptorType::RWTexture2D, 		1024 },
		{ EVkDescriptorType::Texture2D, 		1024 },
	}};

	FVkDescriptorSetCreateInfo DescriptorSetCreateInfo =
	{
		DescriptorPool,
		DescriptorSetLayout
	};

	FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = 
	{
		{
    		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,	16.0f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 	16.0f },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 	16.0f },
		},
		1,
		VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT
	};

	FVkPipelineLayoutCreateInfo PipelineLayoutCreateInfo =
	{
		{ 
			DescriptorSetLayout
		}
	};
	
	FVkBufferCreateInfo GlobalBufferCreateInfo = 
	{ 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
		VMA_MEMORY_USAGE_CPU_TO_GPU, 
		64 
	};
	
	FVkBufferCreateInfo CameraBufferCreateInfo = 
	{ 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
		VMA_MEMORY_USAGE_CPU_TO_GPU, 
		160 * 4 
	};
	
	FVkBufferCreateInfo MaterialBufferCreateInfo = 
	{
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
		VMA_MEMORY_USAGE_CPU_TO_GPU, 
		64 * 1024
	};
	
	FVkBufferCreateInfo InstanceBufferCreateInfo = 
	{ 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
		VMA_MEMORY_USAGE_CPU_TO_GPU, 
		128 * 1024 * 1024 
	};

	FVkImageCreateInfo ToneMappingHDR16CreateInfo = 
	{ 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		GetSwapchain()->Info.SwapchainImageExtent,
		VK_FORMAT_R16G16B16A16_SFLOAT
	};

	FVkImageCreateInfo ToneMappingLDR8CreateInfo = 
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		GetSwapchain()->Info.SwapchainImageExtent,
		VK_FORMAT_R8G8B8A8_UNORM
	};

	DescriptorPool->Initialize(DescriptorPoolCreateInfo);
	DescriptorSetLayout->Initialize(DescriptorSetLayoutCreateInfo);
	DescriptorSet->Initialize(DescriptorSetCreateInfo);

	PipelineLayout->Initialize(PipelineLayoutCreateInfo);

	for (SizeType FrameIndex = 0; FrameIndex < CONCURRENT_FRAME_COUNT; ++FrameIndex)
	{
		GlobalBuffer[FrameIndex] = new FVkBuffer();
		CameraBuffer[FrameIndex] = new FVkBuffer();
		MaterialBuffer[FrameIndex] = new FVkBuffer();
		InstanceBuffer[FrameIndex] = new FVkBuffer();

		HDRImage[FrameIndex] = new FVkImage();
		SDRImage[FrameIndex] = new FVkImage();
		
		GlobalBuffer[FrameIndex]->Initialize(GlobalBufferCreateInfo);
		CameraBuffer[FrameIndex]->Initialize(CameraBufferCreateInfo);
		MaterialBuffer[FrameIndex]->Initialize(MaterialBufferCreateInfo);
		InstanceBuffer[FrameIndex]->Initialize(InstanceBufferCreateInfo);
		
		HDRImage[FrameIndex]->Initialize(ToneMappingHDR16CreateInfo);
		SDRImage[FrameIndex]->Initialize(ToneMappingLDR8CreateInfo);
	}

	Buffers.Insert("Global", GlobalBuffer);
	Buffers.Insert("Camera", CameraBuffer);
	Buffers.Insert("Material", MaterialBuffer);
	Buffers.Insert("Instance", InstanceBuffer);

	RWTexture2D.Insert("HDR", HDRImage);
	RWTexture2D.Insert("SDR", SDRImage);

	FVkOpaqueScriptableRendererPipeline* OpaqueSRP = new FVkOpaqueScriptableRendererPipeline();
	OpaqueSRP->Initialize(PipelineLayout);
	ScriptableRendererPipelineData.Add(OpaqueSRP);

	FVkPostProcessScriptableRendererPipeline* PostProcessSRP = new FVkPostProcessScriptableRendererPipeline();
	PostProcessSRP->Initialize(PipelineLayout);
	ScriptableRendererPipelineData.Add(PostProcessSRP);

	FVkOverlayScriptableRendererPipeline* OverlaySRP = new FVkOverlayScriptableRendererPipeline();
	OverlaySRP->Initialize(PipelineLayout);
	ScriptableRendererPipelineData.Add(OverlaySRP);
}

void FVkForwardRenderer::Shutdown() 
{
    Super::Shutdown();

	vkDestroyPipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), PipelineLayout->Info.Handle, VK_NULL_HANDLE);

	for (SizeType Index = 0; Index < CONCURRENT_FRAME_COUNT; ++Index)
	{
		for (const auto& Buffer : Buffers)
		{
			Buffer.Value[Index]->Free();
		}

		for (const auto& Image : RWTexture2D)
		{
			Image.Value[Index]->Shutdown();
		}
	}

	DescriptorSet->Shutdown();
	DescriptorSetLayout->Shutdown();
	DescriptorPool->Shutdown();

	for (const auto& Pair : ScriptableRendererPipelineData)
	{
		Pair->Shutdown();
	}
}

void FVkForwardRenderer::Bind()
{
	PROFILE_FUNC_SCOPE("FVkForwardRenderer::Bind")

	vkCmdBindDescriptorSets(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout->Info.Handle, 0, 1, &DescriptorSet->Info.Handle, 0, 0);
	vkCmdBindDescriptorSets(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, PipelineLayout->Info.Handle, 0, 1, &DescriptorSet->Info.Handle, 0, 0);

	DescriptorSet->WriteBuffer(0, 0, (*Buffers.Find("Global"))[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteBuffer(0, 1, (*Buffers.Find("Camera"))[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteBuffer(0, 2, (*Buffers.Find("Material"))[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteBuffer(0, 3, (*Buffers.Find("Instance"))[GetRHI()->GetRenderer()->GetFrameIndex()]);

	DescriptorSet->WriteImage(1, 0, (*RWTexture2D.Find("HDR"))[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteImage(1, 1, (*RWTexture2D.Find("SDR"))[GetRHI()->GetRenderer()->GetFrameIndex()]);

	{
		struct FGlobalData
		{
    		float DeltaTime;
		} GlobalData;
		GlobalData.DeltaTime = GetEngine()->Timestep.GetDeltaTime();
		
		(*Buffers.Find("Global"))[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(&GlobalData, sizeof(FGlobalData));
	}

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

		(*Buffers.Find("Camera"))[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(&CameraData, sizeof(FCameraData));
	}

	{
		struct FMaterialData
		{
			alignas(4) uint32 AlbedoTextureID;
    		alignas(4) uint32 NormalTextureID;
		};

		TArray<FMaterialData> MaterialData;
		GetScene()->GetRegistry()->View<FMaterialComponent>([&](const FMaterialComponent& MaterialComponent)
		{
			uint32 AlbedoTextureID = 0;
			uint32 NormalTextureID = 0;

			FMaterialData Material = { .AlbedoTextureID = AlbedoTextureID, .NormalTextureID = NormalTextureID };
			MaterialData.Add(Material);			
		});
		(*Buffers.Find("Material"))[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(MaterialData.GetData(), sizeof(FMaterialData) * MaterialData.GetSize());
	}

	{
		struct FInstanceData
		{
			alignas(16) glm::mat4 Transform;
			alignas(16) glm::mat4 TransformInverseTranspose;
		};

		TArray<FInstanceData> InstanceData;
		GetScene()->GetRegistry()->View<FTransformComponent, FMeshComponent>([&](const FTransformComponent& TransformComponent, const FMeshComponent& MeshComponent)
		{
			glm::mat4 Transform = TransformComponent.Transform.ToMatrix();
			glm::mat4 TransformInverseTranspose = glm::inverseTranspose(TransformComponent.Transform.ToMatrix());

			FInstanceData Instance = { .Transform = Transform, .TransformInverseTranspose = TransformInverseTranspose };
			InstanceData.Add(Instance);
		});

		(*Buffers.Find("Instance"))[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(InstanceData.GetData(), sizeof(FInstanceData) * InstanceData.GetSize());
	}
}

void FVkForwardRenderer::BindImGui()
{
}