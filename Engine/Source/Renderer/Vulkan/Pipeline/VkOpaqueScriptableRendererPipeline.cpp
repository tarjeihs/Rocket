#include "EnginePCH.h"
#include "VkOpaqueScriptableRendererPipeline.h"

#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VkSceneBuffer.h"

void FVkOpaqueScriptableRendererPipeline::Initialize(FVkPipelineLayout* PipelineLayout)
{
	TUniquePtr<FVkShader> VertexShader = MakeUnique<FVkShader>();
	TUniquePtr<FVkShader> PixelShader = MakeUnique<FVkShader>();

	FShaderCreateInfo VertexShaderCreateInfo
	{
		RK_ENGINE_DIR "/Shaders/HLSL/OpaqueVS.hlsl", EShaderStage::Vertex
	};

	FShaderCreateInfo PixelShaderCreateInfo
	{
		RK_ENGINE_DIR "/Shaders/HLSL/OpaquePS.hlsl", EShaderStage::Pixel
	};

	// TODO: Add more customization options here...
	FVkPipelineCreateInfo PipelineCreateInfo
	{
		{
			VertexShader.Get(), 
			PixelShader.Get()
		},
		PipelineLayout
	};

	VertexShader->Init(VertexShaderCreateInfo);
	PixelShader->Init(PixelShaderCreateInfo);

	Pipeline = MakeUnique<FVkPipeline>();
	Pipeline->Initialize(PipelineCreateInfo);

    VertexShader->Free();
    PixelShader->Free();
}

void FVkOpaqueScriptableRendererPipeline::Shutdown()
{
	Pipeline->Shutdown();
}

void FVkOpaqueScriptableRendererPipeline::Bind()
{
    PVulkanCommandBuffer* CommandBuffer = GetRHI()->GetRenderer()->GetCommandBuffer();
    FVkSceneBuffer* SceneBuffer = GetRHI()->GetRenderer()->GetSceneBuffer();
    
    vkCmdBindPipeline(CommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->Info.Handle);

	VkDeviceSize Offsets[] = { 0 };

	vkCmdBindVertexBuffers(CommandBuffer->GetVkCommandBuffer(), 0, 1, &SceneBuffer->VertexBuffer->Info.Handle, Offsets);
	vkCmdBindIndexBuffer(CommandBuffer->GetVkCommandBuffer(), SceneBuffer->IndexBuffer->Info.Handle, 0, VK_INDEX_TYPE_UINT32);

	SceneBuffer->DrawIndexedIndirect(CommandBuffer);
}