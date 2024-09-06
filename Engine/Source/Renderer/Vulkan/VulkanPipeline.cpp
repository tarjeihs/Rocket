#include "VulkanPipeline.h"

#include <cstdlib>

#include "EngineMacros.h"
#include "Renderer/RHI.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "VulkanFrame.h"
#include "Core/Assert.h"
#include "Core/Camera.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Common/Mesh.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanMemory.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Scene/Scene.h"

void PVulkanPipelineLayout::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& DescriptorSetLayouts, const std::vector<VkPushConstantRange>& PushConstantRanges)
{
    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo{};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.pNext = nullptr;
    PipelineLayoutCreateInfo.flags = 0;
    PipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(DescriptorSetLayouts.size());
    PipelineLayoutCreateInfo.pSetLayouts = DescriptorSetLayouts.data();
    PipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(PushConstantRanges.size());
    PipelineLayoutCreateInfo.pPushConstantRanges = PushConstantRanges.data();

    VkResult Result = vkCreatePipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), &PipelineLayoutCreateInfo, nullptr, &PipelineLayout);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create pipeline layout.");
}

void PVulkanPipelineLayout::DestroyPipelineLayout()
{
    vkDestroyPipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), PipelineLayout, nullptr);
}

VkPipelineLayout PVulkanPipelineLayout::GetVkPipelineLayout() const
{
    return PipelineLayout;
}

void PVulkanGraphicsPipeline::CreatePipeline()
{
    VertexShader = new PVulkanShader();
    VertexShader->CreateShader(WIDEN(RK_ENGINE_DIR) L"/Shaders/HLSL/Vertex.hlsl", L"main", "vs_6_0");

    FragmentShader = new PVulkanShader();
    FragmentShader->CreateShader(WIDEN(RK_ENGINE_DIR) L"/Shaders/HLSL/Pixel.hlsl", L"main", "ps_6_0");

    constexpr size_t StorageBufferSize = sizeof(SShaderStorageBufferObject);
    StorageBuffer = new SVulkanBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    StorageBuffer->Allocate(StorageBufferSize);

    constexpr size_t UniformBufferSize = sizeof(SUniformBufferObject);
    UniformBuffer = new SVulkanBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    UniformBuffer->Allocate(UniformBufferSize);

    VkPipelineShaderStageCreateInfo VertexShaderStageCreateInfo{};
    VertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertexShaderStageCreateInfo.pNext = nullptr;
    VertexShaderStageCreateInfo.pName = "main";
    VertexShaderStageCreateInfo.module = VertexShader->GetVkShaderModule();
    VertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineShaderStageCreateInfo FragmentShaderStageCreateInfo{};
    FragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragmentShaderStageCreateInfo.pNext = nullptr;
    FragmentShaderStageCreateInfo.pName = "main";
    FragmentShaderStageCreateInfo.module = FragmentShader->GetVkShaderModule();
    FragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::vector<PVulkanDescriptorSetLayout::EDescriptorSetLayoutType> LayoutTypes =
    {
        PVulkanDescriptorSetLayout::EDescriptorSetLayoutType::Storage,
        PVulkanDescriptorSetLayout::EDescriptorSetLayoutType::Uniform,
    };

    DescriptorSetLayout = new PVulkanDescriptorSetLayout();
    DescriptorSetLayout->CreateDescriptorSetLayout(LayoutTypes);

    DescriptorSet = new PVulkanDescriptorSet();
    DescriptorSet->CreateDescriptorSet(DescriptorSetLayout);
    DescriptorSet->UseDescriptorStorageBuffer(StorageBuffer, 0, VK_WHOLE_SIZE, 0);
    DescriptorSet->UseDescriptorUniformBuffer(UniformBuffer, 0, sizeof(SUniformBufferObject), 1);

    VkPushConstantRange UInt64PointerPushConstantRange{};
    UInt64PointerPushConstantRange.offset = 0;
    UInt64PointerPushConstantRange.size = sizeof(SUInt64PointerPushConstant);
    UInt64PointerPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    PipelineLayout = new PVulkanPipelineLayout(); 
    PipelineLayout->CreatePipelineLayout(
        { DescriptorSetLayout->GetVkDescriptorSetLayout() }, 
        { UInt64PointerPushConstantRange }
    );

    VkFormat ColorAttachmentFormat = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetVkFormat();
    std::vector<VkDynamicState> DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos = { VertexShaderStageCreateInfo, FragmentShaderStageCreateInfo };

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo{};
    InputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo{};
    RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    RasterizationStateCreateInfo.lineWidth = 1.0f;
    RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo{};
    MultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisampleStateCreateInfo.minSampleShading = 1.0f;
    MultisampleStateCreateInfo.pSampleMask = nullptr;
    MultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    MultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState ColorBlendAttachmentState{};
    ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachmentState.blendEnable = VK_TRUE;
    ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo{};
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

    VkPipelineRenderingCreateInfo RenderingCreateInfo{};
    RenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    RenderingCreateInfo.colorAttachmentCount = 1;
    RenderingCreateInfo.pColorAttachmentFormats = &ColorAttachmentFormat;
    RenderingCreateInfo.depthAttachmentFormat = GetRHI()->GetSceneRenderer()->GetDepthImage()->GetVkFormat();

    VkPipelineViewportStateCreateInfo ViewportStateCreateInfo{};
    ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportStateCreateInfo.pNext = nullptr;
    ViewportStateCreateInfo.viewportCount = 1;
    ViewportStateCreateInfo.scissorCount = 1;

    // No color blending. This setup allows the pipeline to be complete and valid according to Vulkan's requirements while giving us the flexibility to introduce actual color blending in the future if needed.
    VkPipelineColorBlendStateCreateInfo ColorBlendingCreateInfo{};
    ColorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendingCreateInfo.pNext = nullptr;
    ColorBlendingCreateInfo.logicOpEnable = VK_FALSE;
    ColorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendingCreateInfo.attachmentCount = 1;
    ColorBlendingCreateInfo.pAttachments = &ColorBlendAttachmentState;

    // Clear VkPipelineVertexInputStateCreateInfo, as we have no need for it in this current moment.
    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo{};
    VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo{};
    DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
    DynamicStateCreateInfo.pDynamicStates = DynamicStates.data();

    VkGraphicsPipelineCreateInfo PipelineCreateInfo{};
    PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCreateInfo.pNext = &RenderingCreateInfo;
    PipelineCreateInfo.stageCount = static_cast<uint32_t>(ShaderStageCreateInfos.size());
    PipelineCreateInfo.pStages = ShaderStageCreateInfos.data();
    PipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;
    PipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
    PipelineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
    PipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
    PipelineCreateInfo.pColorBlendState = &ColorBlendingCreateInfo;
    PipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;
    PipelineCreateInfo.layout = PipelineLayout->GetVkPipelineLayout();
    PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;

    VkResult Result = vkCreateGraphicsPipelines(GetRHI()->GetDevice()->GetVkDevice(), VK_NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &Pipeline);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create graphics pipeline.");
}

void PVulkanGraphicsPipeline::DestroyPipeline()
{
    VertexShader->DestroyShader();
    FragmentShader->DestroyShader();
    DescriptorSetLayout->FreeDescriptorSetLayout();
    DescriptorSet->FreeDescriptorSet();
    PipelineLayout->DestroyPipelineLayout();
    //GetRHI()->GetMemory()->FreeBuffer(UniformBuffer);
    //GetRHI()->GetMemory()->FreeBuffer(StorageBuffer);
    UniformBuffer->Free();
    StorageBuffer->Free();
    vkDestroyPipeline(GetRHI()->GetDevice()->GetVkDevice(), Pipeline, nullptr);

    delete VertexShader;
    delete FragmentShader;
    delete DescriptorSetLayout;
    delete DescriptorSet;
    delete PipelineLayout;
    delete UniformBuffer;
    delete StorageBuffer;
}

void PVulkanGraphicsPipeline::Bind(STransform Transform)
//void PVulkanGraphicsPipeline::Bind()
{
    PVulkanFrame* Frame = GetRHI()->GetSceneRenderer()->GetFramePool()->Pool[GetRHI()->GetSceneRenderer()->GetFramePool()->FrameIndex % 2];

    PCamera* Camera = GetScene()->GetCamera();
    SUniformBufferObject UBO;
    UBO.ViewMatrix = Camera->GetViewMatrix();
    UBO.ProjectionMatrix = Camera->GetProjectionMatrix();
    UBO.WorldMatrix = Transform.ToMatrix();
    void* Data;
    vmaMapMemory(GetRHI()->GetMemory()->GetMemoryAllocator(), UniformBuffer->Allocation, &Data);
    memcpy(Data, &UBO, sizeof(SUniformBufferObject));
    vmaUnmapMemory(GetRHI()->GetMemory()->GetMemoryAllocator(), UniformBuffer->Allocation);

    VkDescriptorSet DescriptorSetPointer = DescriptorSet->GetVkDescriptorSet();

    VkRenderingAttachmentInfo ColorAttachment{};
    ColorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    ColorAttachment.pNext = nullptr;
    ColorAttachment.imageView = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetVkImageView();
    ColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

    VkRenderingAttachmentInfo DepthAttachment{};
    DepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    DepthAttachment.pNext = nullptr;
    DepthAttachment.imageView = GetRHI()->GetSceneRenderer()->GetDepthImage()->GetVkImageView();
    DepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthAttachment.clearValue.depthStencil.depth = 1.0f;

    VkRenderingInfo RenderingInfo{};
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    RenderingInfo.pNext = nullptr;
    RenderingInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, VkExtent2D { GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().width, GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().height }};
    RenderingInfo.layerCount = 1;
    RenderingInfo.colorAttachmentCount = 1;
    RenderingInfo.pColorAttachments = &ColorAttachment;
    RenderingInfo.pDepthAttachment = &DepthAttachment;
    RenderingInfo.pStencilAttachment = nullptr;

    VkViewport Viewport{};
    Viewport.x = 0;
    Viewport.y = 0;
    Viewport.width = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().width;
    Viewport.height = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    VkRect2D Scissor = {};
    Scissor.offset.x = 0;
    Scissor.offset.y = 0;
    Scissor.extent.width = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().width;
    Scissor.extent.height = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetImageExtent2D().height;

    vkCmdBeginRendering(Frame->CommandBuffer->GetVkCommandBuffer(), &RenderingInfo);
    vkCmdSetViewport(Frame->CommandBuffer->GetVkCommandBuffer(), 0, 1, &Viewport);
    vkCmdSetScissor(Frame->CommandBuffer->GetVkCommandBuffer(), 0, 1, &Scissor);

    vkCmdBindPipeline(Frame->CommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
    vkCmdBindDescriptorSets(Frame->CommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout->GetVkPipelineLayout(), 0, 1, &DescriptorSetPointer, 0, nullptr);
}

void PVulkanGraphicsPipeline::Unbind()
{
    PVulkanFrame* Frame = GetRHI()->GetSceneRenderer()->GetFramePool()->Pool[GetRHI()->GetSceneRenderer()->GetFramePool()->FrameIndex % 2];
    vkCmdEndRendering(Frame->CommandBuffer->GetVkCommandBuffer());
}

PVulkanPipelineLayout * PVulkanGraphicsPipeline::GetPipelineLayout() const
{
    return PipelineLayout;
}

VkPipeline PVulkanGraphicsPipeline::GetVkPipeline() const
{
    return Pipeline;
}
