#include "EnginePCH.h"
#include "VkForwardRenderer.h"

#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/Pipeline/VkOpaqueScriptableRendererPipeline.h"

void FVkForwardRenderer::Init()
{
    Super::Init();

	FVkPipelineLayout*			PipelineLayout			= new FVkPipelineLayout();

	FVkDescriptorSetLayout* 	DescriptorSetLayout 	= new FVkDescriptorSetLayout();
	FVkDescriptorSet* 			DescriptorSet 			= new FVkDescriptorSet();
	FVkDescriptorPool* 			DescriptorPool 			= new FVkDescriptorPool();

	FVkBuffer** 				GlobalBuffer 			= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				CameraBuffer 			= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				MaterialBuffer 			= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];
	FVkBuffer** 				InstanceBuffer 			= new FVkBuffer*			[CONCURRENT_FRAME_COUNT];

	FVkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = 
	{{
		{ EVkDescriptorType::SSBO, 1 },
		{ EVkDescriptorType::SSBO, 1 },
		{ EVkDescriptorType::SSBO, 1 },
		{ EVkDescriptorType::SSBO, 1 },
	}};

	FVkDescriptorSetCreateInfo DescriptorSetCreateInfo =
	{
		DescriptorPool,
		DescriptorSetLayout
	};

	FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = 
	{
		{
    		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4.0f },
		},
		1,
		VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT
	};

	FVkPipelineLayoutCreateInfo PipelineLayoutCreateInfo =
	{
		{ DescriptorSetLayout }
	};

	FVkBufferCreateInfo GlobalBufferCreateInfo 		= { VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 64 };
	FVkBufferCreateInfo CameraBufferCreateInfo 		= { VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 128 };
	FVkBufferCreateInfo MaterialBufferCreateInfo 	= { VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 64 * 1024 };
	FVkBufferCreateInfo InstanceBufferCreateInfo 	= { VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 64 * 1024 * 1024 };

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

		GlobalBuffer[FrameIndex]->Initialize(GlobalBufferCreateInfo);
		CameraBuffer[FrameIndex]->Initialize(CameraBufferCreateInfo);
		MaterialBuffer[FrameIndex]->Initialize(MaterialBufferCreateInfo);
		InstanceBuffer[FrameIndex]->Initialize(InstanceBufferCreateInfo);
	}

	DescriptorSet->Info.Buffers.Add(GlobalBuffer);
	DescriptorSet->Info.Buffers.Add(CameraBuffer);
	DescriptorSet->Info.Buffers.Add(MaterialBuffer);
	DescriptorSet->Info.Buffers.Add(InstanceBuffer);

	DescriptorSet->WriteBuffer(0, GlobalBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteBuffer(1, CameraBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteBuffer(2, MaterialBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]);
	DescriptorSet->WriteBuffer(3, InstanceBuffer[GetRHI()->GetRenderer()->GetFrameIndex()]);

	DescriptorPoolData.Add(DescriptorPool);
	DescriptorSetLayoutData.Add(DescriptorSetLayout);
	DescriptorSetData.Add(DescriptorSet);

	FVkOpaqueScriptableRendererPipeline* OpaqueSRP = new FVkOpaqueScriptableRendererPipeline();
	OpaqueSRP->Initialize(PipelineLayout);
	ScriptableRendererPipelineData.Insert("Opaque", OpaqueSRP);

	SharedPipelineLayout = PipelineLayout;
}

void FVkForwardRenderer::Shutdown() 
{
    Super::Shutdown();

	vkDestroyPipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), SharedPipelineLayout->Info.Handle, VK_NULL_HANDLE);

	for (const auto& DescriptorSet : DescriptorSetData)
	{
		DescriptorSet->Destroy();
	}

	for (const auto& DescriptorSetLayout : DescriptorSetLayoutData)
	{
		DescriptorSetLayout->Destroy();
	}

	for (const auto& DescriptorPool : DescriptorPoolData)
	{
		DescriptorPool->Destroy();
	}

	for (const auto& Pair : ScriptableRendererPipelineData)
	{
		Pair.Value->Shutdown();
	}
}

void FVkForwardRenderer::Bind()
{
	PROFILE_FUNC_SCOPE("FVkForwardRenderer::Bind")

	for (const auto& DescriptorSet : DescriptorSetData)
	{
		DescriptorSet->Bind(SharedPipelineLayout);
	}

	for (const auto& Pair : ScriptableRendererPipelineData)
	{
		Pair.Value->Bind();
	}
}