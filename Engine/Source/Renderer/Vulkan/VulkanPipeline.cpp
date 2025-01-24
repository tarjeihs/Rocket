#include "EnginePCH.h"
#include "VulkanPipeline.h"

#include "Renderer/Common/Memory.h"
#include "Renderer/Common/Pipeline.h"
#include "Renderer/RHI.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Types/Vertex.h"

void FVkPipelineLayout::Initialize(const FVkPipelineLayoutCreateInfo& CreateInfo)
{
    TArray<VkDescriptorSetLayout> DescriptorSetLayouts;
    for (SizeType Index = 0; Index < CreateInfo.DescriptorSetLayouts.GetSize(); ++Index)
    {
        DescriptorSetLayouts.Add(CreateInfo.DescriptorSetLayouts[Index]->Info.Handle);
    }

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32>(DescriptorSetLayouts.GetSize());
    PipelineLayoutCreateInfo.pSetLayouts = DescriptorSetLayouts.GetData();

    VkResult Result = vkCreatePipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), &PipelineLayoutCreateInfo, nullptr, &Info.Handle);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create pipeline layout.");
}

void FVkPipelineLayout::Shutdown()
{
   vkDestroyPipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), Info.Handle, VK_NULL_HANDLE); 
}

void FVkPipeline::Shutdown()
{
    vkDestroyPipeline(GetRHI()->GetDevice()->GetVkDevice(), Info.Handle, VK_NULL_HANDLE);
}

void FVkPipelineGfx::Initialize(FVkPipelineCreateInfo& CreateInfo)
{
    RK_ASSERT(CreateInfo.Shaders.GetSize() > 0, "Atleast one graphics shader is required.");
	TArray<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;

	for (const FVkShader* Shader : CreateInfo.Shaders)
	{
		VkPipelineShaderStageCreateInfo ShaderStageCreateInfo = {};
		ShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ShaderStageCreateInfo.pNext = nullptr;
		ShaderStageCreateInfo.pName = "main";
		ShaderStageCreateInfo.module = Shader->Info.Module;
		ShaderStageCreateInfo.stage = Shader->Info.Stage;
		ShaderStageCreateInfos.Add(ShaderStageCreateInfo);
	}

	SizeType Stride = 0;
	TArray<VkVertexInputAttributeDescription> AttributeDescriptions = {};
	TArray<VkDynamicState> DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	for (const FVkVertexAttribute& Attribute : CreateInfo.Attributes)
	{
		VkVertexInputAttributeDescription AttributeDescription;
		AttributeDescription.binding = Attribute.Binding;
		AttributeDescription.location = Attribute.Location;
		AttributeDescription.format = Attribute.Format;
		AttributeDescription.offset = Attribute.Offset;

		Stride = Attribute.Stride;
		AttributeDescriptions.Add(AttributeDescription);
	}

	VkVertexInputBindingDescription VertexInputBindingDescription = {};
	VertexInputBindingDescription.binding = 0;
	VertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VertexInputBindingDescription.stride = Stride;

	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = {};
	VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	VertexInputStateCreateInfo.pVertexBindingDescriptions = &VertexInputBindingDescription;
	VertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(AttributeDescriptions.GetSize());
	VertexInputStateCreateInfo.pVertexAttributeDescriptions = AttributeDescriptions.GetData();

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo = {};
	InputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	InputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo = {};
	RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	RasterizationStateCreateInfo.lineWidth = 1.0f;
	RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo = {};
	MultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	MultisampleStateCreateInfo.minSampleShading = 1.0f;
	MultisampleStateCreateInfo.pSampleMask = nullptr;
	MultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	MultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = {};
	ColorBlendAttachmentState.blendEnable = VK_TRUE;
	ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo = {};
	DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	DepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	DepthStencilStateCreateInfo.front = {};
	DepthStencilStateCreateInfo.back = {};
	DepthStencilStateCreateInfo.minDepthBounds = 0.0f;
	DepthStencilStateCreateInfo.maxDepthBounds = 1.0f;

	VkPipelineRenderingCreateInfo RenderingCreateInfo = {};
	RenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	RenderingCreateInfo.colorAttachmentCount = 1;
	RenderingCreateInfo.pColorAttachmentFormats = &GetRHI()->GetRenderer()->GetColorAttachment16()->Info.Format;
	RenderingCreateInfo.depthAttachmentFormat = GetRHI()->GetRenderer()->GetDepthAttachmentD32()->Info.Format;

	VkPipelineViewportStateCreateInfo ViewportStateCreateInfo = {};
	ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportStateCreateInfo.pNext = nullptr;
	ViewportStateCreateInfo.viewportCount = 1;
	ViewportStateCreateInfo.scissorCount = 1;

	VkPipelineColorBlendStateCreateInfo ColorBlendingCreateInfo = {};
	ColorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlendingCreateInfo.pNext = nullptr;
	ColorBlendingCreateInfo.logicOpEnable = VK_FALSE;
	ColorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	ColorBlendingCreateInfo.attachmentCount = 1;
	ColorBlendingCreateInfo.pAttachments = &ColorBlendAttachmentState;

	VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = {};
	DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	DynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(DynamicStates.GetSize());
	DynamicStateCreateInfo.pDynamicStates = DynamicStates.GetData();

	VkGraphicsPipelineCreateInfo PipelineCreateInfo = {};
	PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	PipelineCreateInfo.pNext = &RenderingCreateInfo;
	PipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
	PipelineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;
	PipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
	PipelineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
	PipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
	PipelineCreateInfo.pColorBlendState = &ColorBlendingCreateInfo;
	PipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;
	PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
	PipelineCreateInfo.pStages = ShaderStageCreateInfos.GetData();
	PipelineCreateInfo.stageCount = static_cast<uint32_t>(ShaderStageCreateInfos.GetSize());
	PipelineCreateInfo.layout = CreateInfo.PipelineLayout->Info.Handle;

	VkResult Result = vkCreateGraphicsPipelines(GetRHI()->GetDevice()->GetVkDevice(), VK_NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &Info.Handle);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create graphics pipeline.");

	RK_LOG_INFO("Gfx Pipeline -> Success");
}

void FVkPipelineGfx::Begin(FVkPipelineBeginInfo& BeginInfo)
{
	FPipelineExecuteInfo ExecuteInfo;
	Info.Pipeline->Execute(ExecuteInfo);

	FVkImage* ColorAttachment = Cast<FVkImage>(GMemory->GetImage(ExecuteInfo.ColorAttachment.Name, GetRHI()->GetRenderer()->GetFrameIndex()));
	FVkImage* DepthAttachment = Cast<FVkImage>(GMemory->GetImage(ExecuteInfo.DepthAttachment.Name, GetRHI()->GetRenderer()->GetFrameIndex()));

	VkRenderingAttachmentInfo ColorAttachmentInfo = {};
	ColorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	ColorAttachmentInfo.pNext = nullptr;
	ColorAttachmentInfo.imageView = ColorAttachment->Info.ImageViewHandle;
	ColorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	ColorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachmentInfo.clearValue = { 0.0033f, 0.0033f, 0.0033f, 1.0f };

	VkRenderingAttachmentInfo DepthAttachmentInfo = {};
	DepthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	DepthAttachmentInfo.pNext = nullptr;
	DepthAttachmentInfo.imageView = DepthAttachment->Info.ImageViewHandle;
	DepthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	DepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	DepthAttachmentInfo.clearValue.depthStencil.depth = 1.0f;

	VkRenderingInfo RenderingInfo = {};
	RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	RenderingInfo.pNext = nullptr;
	RenderingInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, VkExtent2D { GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.width, GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.height } };
	RenderingInfo.layerCount = 1;
	RenderingInfo.colorAttachmentCount = 1;
	RenderingInfo.pColorAttachments = &ColorAttachmentInfo;
	RenderingInfo.pDepthAttachment = &DepthAttachmentInfo;
	RenderingInfo.pStencilAttachment = nullptr;

	VkViewport Viewport = {};
	Viewport.x = 0;
	Viewport.y = 0;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;
	Viewport.width = GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.width;
	Viewport.height = GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.height;

	VkRect2D Scissor = {};
	Scissor.offset.x = 0;
	Scissor.offset.y = 0;
	Scissor.extent.width = GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.width;
	Scissor.extent.height = GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent.height;

	vkCmdBeginRendering(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), &RenderingInfo);
	vkCmdSetViewport(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), 0, 1, &Viewport);
	vkCmdSetScissor(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), 0, 1, &Scissor);
	vkCmdBindPipeline(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Info.Handle);

	//for (const TPair<uint32, FString>& Pair : ExecuteInfo.Buffers)
	//{
	//	IBuffer* Interface = GMemory->GetBuffer(Pair.Value, GetRHI()->GetRenderer()->GetFrameIndex());
	//
	//	FVkBuffer* Buffer = Cast<FVkBuffer>(Interface);
	//	FVkMemory* Memory = Cast<FVkMemory>(GMemory);
	//
	//	Memory->Info.DescriptorSet->WriteBuffer(0, Pair.Key, Buffer);
	//}
	//
	//for (const TPair<uint32, FString>& Pair : ExecuteInfo.Images)
	//{
	//	IImage* Interface = GMemory->GetImage(Pair.Value, GetRHI()->GetRenderer()->GetFrameIndex());
	//
	//	FVkImage* Image = Cast<FVkImage>(Interface);
	//	FVkMemory* Memory = Cast<FVkMemory>(GMemory);
	//
	//	Memory->Info.DescriptorSet->WriteImage(1, Pair.Key, Image);
	//}
}

void FVkPipelineGfx::End(FVkPipelineEndInfo& EndInfo)
{
	vkCmdEndRendering(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer());
}

void FVkPipelineCompute::Initialize(FVkPipelineCreateInfo& CreateInfo)
{
	RK_ASSERT(CreateInfo.Shaders.GetSize() == 1, "Compute pipeline requires exactly one compute shader.");
	const FVkShader* ComputeShader = CreateInfo.Shaders[0];

	VkPipelineShaderStageCreateInfo ShaderStageCreateInfo = {};
	ShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	ShaderStageCreateInfo.pNext = nullptr;
	ShaderStageCreateInfo.stage = ComputeShader->Info.Stage;
	ShaderStageCreateInfo.module = ComputeShader->Info.Module;
	ShaderStageCreateInfo.pName = "main";

	VkComputePipelineCreateInfo PipelineCreateInfo = {};
	PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	PipelineCreateInfo.pNext = nullptr;
	PipelineCreateInfo.flags = 0;
	PipelineCreateInfo.stage = ShaderStageCreateInfo;
	PipelineCreateInfo.layout = CreateInfo.PipelineLayout->Info.Handle;

	VkResult Result = vkCreateComputePipelines(GetRHI()->GetDevice()->GetVkDevice(), VK_NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &Info.Handle);
	RK_ASSERT(Result == VK_SUCCESS, "Failed to create compute pipeline.");

	RK_LOG_INFO("Compute Pipeline -> Success");
}

void FVkPipelineCompute::Begin(FVkPipelineBeginInfo& BeginInfo)
{
	vkCmdBindPipeline(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, Info.Handle);
}

void FVkPipelineCompute::End(FVkPipelineEndInfo& EndInfo)
{
}