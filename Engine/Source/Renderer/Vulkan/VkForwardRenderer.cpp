#include "EnginePCH.h"
#include "VkForwardRenderer.h"

#include "Pipeline/VkPostProcessScriptableRendererPipeline.h"
#include "Pipeline/VkOpaqueSkinnedGfxPipeline.h"
#include "Renderer/RHI.h"
#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/Pipeline/VkOpaqueScriptableRendererPipeline.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Common/Memory.h"
#include "Scene/Scene.h"
#include "Utils/Profiler.h"
#include "VulkanTexture2D.h"

struct FGlobalData
{
	alignas(4) float DeltaTime = 0.0f;
} GlobalData;

struct FCameraData
{
	alignas(16) glm::mat4 View = glm::identity<glm::mat4>();
	alignas(16) glm::mat4 Projection = glm::identity<glm::mat4>();
	alignas(16) glm::vec3 Position = glm::vec3(0.0f);
	alignas(16) glm::vec3 Direction = glm::vec3(0.0f);
	alignas(16) float PP_FilmGrainIntensity = 0.0f;
	alignas(16) float PP_ChromaticAberration = 0.0f;
} CameraData;

struct FMaterialData
{
	alignas(4) int32 AlbedoTextureID;
	alignas(4) int32 NormalTextureID;
	alignas(4) int32 RoughnessTextureID;
	alignas(4) int32 MetallicTextureID;
};

struct FInstanceData
{
	alignas(16) glm::mat4 Transform;
	alignas(16) glm::mat4 TransformInverseTranspose;
};

void FVkForwardRenderer::Init()
{
    Super::Init();

	FVkBuffer** 				GlobalBuffer 					= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				CameraBuffer 					= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				MaterialBuffer 					= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				InstanceBuffer 					= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	
	FVkImage**					HDRImage 						= new FVkImage*				[CONCURRENT_FRAME_COUNT];
	FVkImage**					SDRImage 						= new FVkImage*				[CONCURRENT_FRAME_COUNT];

	FBufferCreateInfo GlobalBufferCreateInfo = 
	{ 
		EBufferUsageFlag::Storage, 
		EBufferTransferFlag::None,
		EBufferMemoryFlag::HostToDevice,
		64 
	};
	
	FBufferCreateInfo CameraBufferCreateInfo = 
	{ 
		EBufferUsageFlag::Storage,
		EBufferTransferFlag::None,
		EBufferMemoryFlag::HostToDevice,
		160 * 4
	};
	
	FBufferCreateInfo MaterialBufferCreateInfo = 
	{
		EBufferUsageFlag::Storage,
		EBufferTransferFlag::None,
		EBufferMemoryFlag::HostToDevice,
		64 * 1024
	};
	
	FBufferCreateInfo InstanceBufferCreateInfo = 
	{ 
		EBufferUsageFlag::Storage,
		EBufferTransferFlag::None,
		EBufferMemoryFlag::HostToDevice,
		128 * 1024 * 1024 
	};

	FImageCreateInfo ToneMappingHDR16CreateInfo =
	{
		EImageLayout::Undefined,
		EImageUsage::Storage | EImageUsage::TransferDst | EImageUsage::TransferSrc,
		EImageAspect::Color,
		{1280,720},
		EImageFormat::R16G16B16A16_SFLOAT
	};

	FImageCreateInfo ToneMappingLDR8CreateInfo =
	{
		EImageLayout::Undefined,
		EImageUsage::Storage | EImageUsage::TransferSrc,
		EImageAspect::Color,
		{1280,720},
		EImageFormat::R8G8B8A8_UNORM
	};

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
		
		HDRImage[FrameIndex] = new FVkImage();
		SDRImage[FrameIndex] = new FVkImage();
		
		HDRImage[FrameIndex]->Initialize(ToneMappingHDR16CreateInfo);
		SDRImage[FrameIndex]->Initialize(ToneMappingLDR8CreateInfo);
	}

	Buffers.Insert("Global", GlobalBuffer);
	Buffers.Insert("Camera", CameraBuffer);
	Buffers.Insert("Material", MaterialBuffer);
	Buffers.Insert("Instance", InstanceBuffer);

	RWTexture2D.Insert("HDR", HDRImage);
	RWTexture2D.Insert("SDR", SDRImage);

	auto* OpaquePipeline = new FVkOpaqueGfxPipeline();
	auto* SkinningPipeline = new FVkOpaqueSkinnedMeshGfxPipeline();
	auto* PostProcessPipeline = new FVkPostProcessComputePipeline();

	OpaquePipeline->Initialize(PipelineLayout);
	SkinningPipeline->Initialize(PipelineLayout);
	PostProcessPipeline->Initialize(PipelineLayout);
	
	ScriptableRendererPipelineData = 
	{
		OpaquePipeline,
		SkinningPipeline,
		PostProcessPipeline,
	};
}

void FVkForwardRenderer::Shutdown() 
{
    Super::Shutdown();

	for (SizeType Index = 0; Index < CONCURRENT_FRAME_COUNT; ++Index)
	{
		for (const auto& Buffer : Buffers)
		{
			Buffer.Value[Index]->Shutdown();
		}

		for (const auto& Image : RWTexture2D)
		{
			Image.Value[Index]->Shutdown();
		}
	}

	for (const auto& Pair : ScriptableRendererPipelineData)
	{
		Pair->Shutdown();
	}
}

void FVkForwardRenderer::Resize()
{
	Super::Resize();
	
	//FVkImageCreateInfo ToneMappingHDR16CreateInfo = 
	//{ 
	//	VK_IMAGE_LAYOUT_UNDEFINED, 
	//	VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	//	VK_IMAGE_ASPECT_COLOR_BIT,
	//	Swapchain->Info.SwapchainImageExtent,
	//	VK_FORMAT_R16G16B16A16_SFLOAT
	//};
	//
	//FVkImageCreateInfo ToneMappingLDR8CreateInfo = 
	//{
	//	VK_IMAGE_LAYOUT_UNDEFINED,
	//	VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	//	VK_IMAGE_ASPECT_COLOR_BIT,
	//	Swapchain->Info.SwapchainImageExtent,
	//	VK_FORMAT_R8G8B8A8_UNORM
	//};

	FImageCreateInfo ToneMappingHDR16CreateInfo =
	{
		EImageLayout::Undefined,
		EImageUsage::Storage | EImageUsage::TransferDst | EImageUsage::TransferSrc,
		EImageAspect::Color,
		{1280,720},
		EImageFormat::R16G16B16A16_SFLOAT
	};

	FImageCreateInfo ToneMappingLDR8CreateInfo =
	{
		EImageLayout::Undefined,
		EImageUsage::Storage | EImageUsage::TransferSrc,
		EImageAspect::Color,
		{1280,720},
		EImageFormat::R8G8B8A8_UNORM
	};

	FVkImage** HDR = *RWTexture2D.Find("HDR");
	FVkImage** SDR = *RWTexture2D.Find("SDR");

	for (SizeType Index = 0; Index < CONCURRENT_FRAME_COUNT; ++Index)
	{
		HDR[Index]->Shutdown();
		HDR[Index]->Initialize(ToneMappingHDR16CreateInfo);

		SDR[Index]->Shutdown();
		SDR[Index]->Initialize(ToneMappingLDR8CreateInfo);
	}
}

void FVkForwardRenderer::Bind()
{
	PROFILE_FUNC_SCOPE("FVkForwardRenderer::Bind")

	DescriptorSet->WriteBuffer(0, 0, (*Buffers.Find("Global"))[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteBuffer(0, 1, (*Buffers.Find("Camera"))[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteBuffer(0, 2, (*Buffers.Find("Material"))[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteBuffer(0, 3, (*Buffers.Find("Instance"))[GetRHI()->GetRenderer()->GetFrameIndex()]);

	DescriptorSet->WriteImage(1, 0, (*RWTexture2D.Find("HDR"))[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteImage(1, 1, (*RWTexture2D.Find("SDR"))[GetRHI()->GetRenderer()->GetFrameIndex()]);

	GlobalData.DeltaTime = GetEngine()->Timestep.GetDeltaTime();

	CameraData.View = GetScene()->GetCamera()->GetViewMatrix();
	CameraData.Projection = GetScene()->GetCamera()->GetProjectionMatrix();
	CameraData.Position = GetScene()->GetCamera()->GetPosition();
	CameraData.Direction = GetScene()->GetCamera()->GetRotation();
	CameraData.PP_FilmGrainIntensity = GetScene()->GetCamera()->Settings.FilmGrainIntensity;
	CameraData.PP_ChromaticAberration = GetScene()->GetCamera()->Settings.ChromaticAberration;

	TArray<FMaterialData> MaterialData;
	GetScene()->GetRegistry()->View<FMaterialComponent>([&](const FMaterialComponent& MaterialComponent)
	{
		int32 AlbedoTextureID = MaterialComponent.Material.AlbedoID;
		int32 NormalTextureID = MaterialComponent.Material.NormalID;
		int32 RoughnessTextureID = MaterialComponent.Material.RoughnessID;
		int32 MetallicTextureID = MaterialComponent.Material.MetallicID;

		FMaterialData Material = { AlbedoTextureID, NormalTextureID, RoughnessTextureID, MetallicTextureID };
		MaterialData.Add(Material);			
	});

	TArray<FInstanceData> InstanceData;
	GetScene()->GetRegistry()->View<FTransformComponent, FMeshComponent>([&](const FTransformComponent& TransformComponent, const FMeshComponent& MeshComponent)
	{
		glm::mat4 Transform = TransformComponent.Transform.ToMatrix();
		glm::mat4 TransformInverseTranspose = glm::inverseTranspose(TransformComponent.Transform.ToMatrix());

		FInstanceData Instance = { .Transform = Transform, .TransformInverseTranspose = TransformInverseTranspose };
		InstanceData.Add(Instance);
	});

	(*Buffers.Find("Global"))[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(&GlobalData, sizeof(FGlobalData));
	(*Buffers.Find("Camera"))[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(&CameraData, sizeof(FCameraData));
	(*Buffers.Find("Material"))[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(MaterialData.GetData(), sizeof(FMaterialData) * MaterialData.GetSize());
	(*Buffers.Find("Instance"))[GetRHI()->GetRenderer()->GetFrameIndex()]->Submit(InstanceData.GetData(), sizeof(FInstanceData) * InstanceData.GetSize());
}

void FVkForwardRenderer::BindImGui()
{
}