#include "VulkanPipeline.h"
#include <Core/Assert.h>
#include "VulkanShader.h"
#include <vk_mem_alloc.h>

void PVulkanPipeline::Init(VkDevice LogicalDevice, VkImageView DrawImageView, VkDescriptorPool DescriptorPool, VkFormat ImageFormat)
{
}

void PVulkanPipeline::Destroy(VkDevice LogicalDevice)
{
	Free(LogicalDevice);
}

void PVulkanPipeline::Free(VkDevice LogicalDevice)
{
	if (PipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(LogicalDevice, PipelineLayout, nullptr);
		Pipeline = VK_NULL_HANDLE;
	}
	
	if (Pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);
		PipelineLayout = VK_NULL_HANDLE;
	}
	
	if (DescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(LogicalDevice, DescriptorSetLayout, nullptr);
		DescriptorSetLayout = VK_NULL_HANDLE;
	}
}

void PVulkanColorGraphicsPipeline::Init(VkDevice LogicalDevice, VkImageView DrawImageView, VkDescriptorPool DescriptorPool, VkFormat ColorAttachmentFormat)
{
	Super::Init(LogicalDevice, DrawImageView, DescriptorPool, ColorAttachmentFormat);

	static bool bCompiled = false;
	if (!bCompiled)
	{
		VertexShader = new PVulkanShader();
		VertexShader->Compile(LogicalDevice, RK_SHADERTYPE_VERTEXSHADER, L"../Engine/Shaders/Mesh.vertex", L"main", "vs_6_0");

		FragmentShader = new PVulkanShader();
		FragmentShader->Compile(LogicalDevice, RK_SHADERTYPE_FRAGMENTSHADER, L"../Engine/Shaders/Mesh.fragment", L"main", "ps_6_0");
		bCompiled = true;
	}

	{
		/* Initialize Descriptor Set & Layout */
		VkDescriptorSetLayoutBinding LayoutBinding{};
		LayoutBinding.binding = 0;
		LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		LayoutBinding.descriptorCount = 1;
		LayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		CreateInfo.bindingCount = 1;
		CreateInfo.pBindings = &LayoutBinding;

		VkResult Result = vkCreateDescriptorSetLayout(LogicalDevice, &CreateInfo, nullptr, &DescriptorSetLayout);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor set layout.");

		VkDescriptorSetAllocateInfo AllocInfo{};
		AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		AllocInfo.descriptorPool = DescriptorPool;
		AllocInfo.descriptorSetCount = 1;
		AllocInfo.pSetLayouts = &DescriptorSetLayout;
		Result = vkAllocateDescriptorSets(LogicalDevice, &AllocInfo, &DescriptorSet);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor sets.");

		VkDescriptorBufferInfo BufferInfo{};
		BufferInfo.buffer = MeshBuffer.vertexBuffer.buffer;
		BufferInfo.offset = 0;
		BufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet WriteDescriptorSet{};
		WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		WriteDescriptorSet.dstSet = DescriptorSet;
		WriteDescriptorSet.dstBinding = 0;
		WriteDescriptorSet.dstArrayElement = 0;
		WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		WriteDescriptorSet.descriptorCount = 1;
		WriteDescriptorSet.pBufferInfo = &BufferInfo;

		vkUpdateDescriptorSets(LogicalDevice, 1, &WriteDescriptorSet, 0, nullptr);
	}

	{
		/* Create Pipeline Layout and Graphics Pipeline */
		VkPushConstantRange bufferRange{};
		bufferRange.offset = 0;
		bufferRange.size = sizeof(SMeshPushConstant);
		bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo LayoutCreateInfo{};
		LayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		LayoutCreateInfo.pNext = nullptr;
		LayoutCreateInfo.flags = 0;
		LayoutCreateInfo.setLayoutCount = 1;
		LayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;
		LayoutCreateInfo.pushConstantRangeCount = 1;
		LayoutCreateInfo.pPushConstantRanges = &bufferRange;

		VkResult Result = vkCreatePipelineLayout(LogicalDevice, &LayoutCreateInfo, nullptr, &PipelineLayout);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create pipeline layout.");

		std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos = { VertexShader->ShaderProgram.CreateInfo, FragmentShader->ShaderProgram.CreateInfo };

		VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo = {};
		InputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		InputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo RasterizerStateCreateInfo = {};
		RasterizerStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		RasterizerStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		RasterizerStateCreateInfo.lineWidth = 1.f;
		RasterizerStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
		RasterizerStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

		VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo = {};
		MultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
		MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		MultisampleStateCreateInfo.minSampleShading = 1.0f;
		MultisampleStateCreateInfo.pSampleMask = nullptr;
		MultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
		MultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = {};
		ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		ColorBlendAttachmentState.blendEnable = VK_TRUE;
		ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo = {};
		DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
		DepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
		DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_NEVER;
		DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
		DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
		DepthStencilStateCreateInfo.front = {};
		DepthStencilStateCreateInfo.back = {};
		DepthStencilStateCreateInfo.minDepthBounds = 0.f;
		DepthStencilStateCreateInfo.maxDepthBounds = 1.f;

		VkPipelineRenderingCreateInfo RenderingCreateInfo = {};
		RenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
		RenderingCreateInfo.colorAttachmentCount = 1;
		RenderingCreateInfo.pColorAttachmentFormats = &ColorAttachmentFormat;
		RenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

		VkPipelineViewportStateCreateInfo ViewportStateCreateInfo = {};
		ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportStateCreateInfo.pNext = nullptr;
		ViewportStateCreateInfo.viewportCount = 1;
		ViewportStateCreateInfo.scissorCount = 1;

		// setup dummy color blending. We arent using transparent objects yet
		// the blending is just "no blend", but we do write to the color attachment
		VkPipelineColorBlendStateCreateInfo ColorBlendingCreateInfo = {};
		ColorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ColorBlendingCreateInfo.pNext = nullptr;
		ColorBlendingCreateInfo.logicOpEnable = VK_FALSE;
		ColorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		ColorBlendingCreateInfo.attachmentCount = 1;
		ColorBlendingCreateInfo.pAttachments = &ColorBlendAttachmentState;

		// completely clear VertexInputStateCreateInfo, as we have no need for it
		VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = {};
		VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkDynamicState DynamicState[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = {};
		DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		DynamicStateCreateInfo.pDynamicStates = DynamicState;
		DynamicStateCreateInfo.dynamicStateCount = 2;

		VkGraphicsPipelineCreateInfo PipelineCreateInfo = {};
		PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		PipelineCreateInfo.pNext = &RenderingCreateInfo;
		PipelineCreateInfo.stageCount = static_cast<uint32_t>(ShaderStageCreateInfos.size());
		PipelineCreateInfo.pStages = ShaderStageCreateInfos.data();
		PipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
		PipelineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;
		PipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
		PipelineCreateInfo.pRasterizationState = &RasterizerStateCreateInfo;
		PipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
		PipelineCreateInfo.pColorBlendState = &ColorBlendingCreateInfo;
		PipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;
		PipelineCreateInfo.layout = PipelineLayout;
		PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;

		Result = vkCreateGraphicsPipelines(LogicalDevice, VK_NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &Pipeline);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create graphics pipeline.");
	}
}

void PVulkanColorGraphicsPipeline::Draw(VkCommandBuffer CommandBuffer, VkImageView DrawImageView, VkExtent2D DrawImageExtent)
{
	VkRenderingAttachmentInfo ColorAttachment = {};
	ColorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	ColorAttachment.pNext = nullptr;
	ColorAttachment.imageView = DrawImageView;
	ColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	VkRenderingInfo RenderingInfo = {};
	RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	RenderingInfo.pNext = nullptr;
	RenderingInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, DrawImageExtent };
	RenderingInfo.layerCount = 1;
	RenderingInfo.colorAttachmentCount = 1;
	RenderingInfo.pColorAttachments = &ColorAttachment;
	RenderingInfo.pDepthAttachment = nullptr;
	RenderingInfo.pStencilAttachment = nullptr;

	VkViewport Viewport = {};
	Viewport.x = 0;
	Viewport.y = 0;
	Viewport.width = DrawImageExtent.width;
	Viewport.height = DrawImageExtent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;

	VkRect2D Scissor = {};
	Scissor.offset.x = 0;
	Scissor.offset.y = 0;
	Scissor.extent.width = DrawImageExtent.width;
	Scissor.extent.height = DrawImageExtent.height;

	vkCmdBeginRendering(CommandBuffer, &RenderingInfo);
	vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);
	vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);

	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
	vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &DescriptorSet, 0, nullptr);

	SMeshPushConstant MeshPushConstant;
	MeshPushConstant.worldMatrix = glm::mat4(1.0f);
	MeshPushConstant.vertexBuffer = MeshBuffer.vertexBufferAddress;
	vkCmdPushConstants(CommandBuffer, PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SMeshPushConstant), &MeshPushConstant);

	vkCmdBindIndexBuffer(CommandBuffer, MeshBuffer.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(CommandBuffer, 6, 1, 0, 0, 0);

	vkCmdEndRendering(CommandBuffer);
}

void PVulkanColorGraphicsPipeline::Destroy(VkDevice LogicalDevice)
{
	Super::Destroy(LogicalDevice);

	vkDestroyShaderModule(LogicalDevice, VertexShader->ShaderProgram.ShaderModule, nullptr);
	vkDestroyShaderModule(LogicalDevice, FragmentShader->ShaderProgram.ShaderModule, nullptr);
}

void PVulkanColorGraphicsPipeline::Free(VkDevice LogicalDevice)
{
	Super::Free(LogicalDevice);

	vmaDestroyBuffer(Allocator, MeshBuffer.indexBuffer.buffer, MeshBuffer.indexBuffer.allocation);
	vmaDestroyBuffer(Allocator, MeshBuffer.vertexBuffer.buffer, MeshBuffer.vertexBuffer.allocation);
}

void PVulkanStandardComputePipeline::Init(VkDevice LogicalDevice, VkImageView DrawImageView, VkDescriptorPool DescriptorPool, VkFormat ImageFormat)
{
	Super::Init(LogicalDevice, DrawImageView, DescriptorPool, ImageFormat);

	static bool bCompiled = false;
	if (!bCompiled)
	{
		ComputeShader = new PVulkanShader();
		ComputeShader->Compile(LogicalDevice, RK_SHADERTYPE_COMPUTE_SHADER, L"../Engine/Shaders/Gradient.compute", L"main", "cs_6_0");
		bCompiled = true;
	}

	VkDescriptorSetLayoutBinding LayoutBinding{};
	LayoutBinding.binding = 0;
	LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	LayoutBinding.descriptorCount = 1;
	LayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // THIS BE HERE???

	VkDescriptorSetLayoutCreateInfo CreateInfo{};
	CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	CreateInfo.bindingCount = 1;
	CreateInfo.pBindings = &LayoutBinding;

	VkResult Result = vkCreateDescriptorSetLayout(LogicalDevice, &CreateInfo, nullptr, &DescriptorSetLayout);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create descriptor set layout.");

	VkDescriptorSetAllocateInfo AllocInfo{};
	AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocInfo.pNext = nullptr;
	AllocInfo.descriptorPool = DescriptorPool;
	AllocInfo.descriptorSetCount = 1;
	AllocInfo.pSetLayouts = &DescriptorSetLayout;
	
	Result = vkAllocateDescriptorSets(LogicalDevice, &AllocInfo, &DescriptorSet);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to allocate descriptor sets.");

	// Update the descriptor set with the new image view from the swapchain
	VkDescriptorImageInfo ImageInfo{};
	ImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	ImageInfo.imageView = DrawImageView;  // Use the new image view after swapchain recreation

	VkWriteDescriptorSet RenderTargetWrite = {};
	RenderTargetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	RenderTargetWrite.pNext = nullptr;
	RenderTargetWrite.dstBinding = 0;
	RenderTargetWrite.dstSet = DescriptorSet;
	RenderTargetWrite.descriptorCount = 1;
	RenderTargetWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	RenderTargetWrite.pImageInfo = &ImageInfo;

	vkUpdateDescriptorSets(LogicalDevice, 1, &RenderTargetWrite, 0, nullptr);

	// Include the push constant range for the compute shader stage
	VkPushConstantRange PushConstantRange{};
	PushConstantRange.offset = 0;
	PushConstantRange.size = sizeof(SComputePushConstant);
	PushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo{};
	PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineLayoutCreateInfo.pNext = nullptr;
	PipelineLayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;
	PipelineLayoutCreateInfo.setLayoutCount = 1;
	PipelineLayoutCreateInfo.pPushConstantRanges = &PushConstantRange;
	PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	
	Result = vkCreatePipelineLayout(LogicalDevice, &PipelineLayoutCreateInfo, nullptr, &PipelineLayout);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create pipeline layout.");
	
	VkComputePipelineCreateInfo ComputePipelineCreateInfo{};
	ComputePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	ComputePipelineCreateInfo.pNext = nullptr;
	ComputePipelineCreateInfo.layout = PipelineLayout;
	ComputePipelineCreateInfo.stage = ComputeShader->ShaderProgram.CreateInfo;
	
	Result = vkCreateComputePipelines(LogicalDevice, VK_NULL_HANDLE, 1, &ComputePipelineCreateInfo, nullptr, &Pipeline);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create compute pipeline.");
}

void PVulkanStandardComputePipeline::Draw(VkCommandBuffer CommandBuffer, VkImageView DrawImageView, VkExtent2D DrawImageExtent)
{
	Super::Draw(CommandBuffer, DrawImageView, DrawImageExtent);

	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
	vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineLayout, 0, 1, &DescriptorSet, 0, nullptr);

	SComputePushConstant ComputePushConstant;
	ComputePushConstant.data1 = glm::vec4(0, 0, 0, 1);
	ComputePushConstant.data2 = glm::vec4(0, 0, 1, 1);
	vkCmdPushConstants(CommandBuffer, PipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(SComputePushConstant), &ComputePushConstant);
	
	// execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
	vkCmdDispatch(CommandBuffer, std::ceil(DrawImageExtent.width / 16.0), std::ceil(DrawImageExtent.height / 16.0), 1);
}

void PVulkanStandardComputePipeline::Destroy(VkDevice LogicalDevice)
{
	Super::Destroy(LogicalDevice);

	vkDestroyShaderModule(LogicalDevice, ComputeShader->ShaderProgram.ShaderModule, nullptr);
}

void PVulkanStandardComputePipeline::Free(VkDevice LogicalDevice)
{
	Super::Free(LogicalDevice);
}
