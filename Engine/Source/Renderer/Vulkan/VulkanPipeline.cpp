#include "EnginePCH.h"
#include "VulkanPipeline.h"

#include "Renderer/Common/Mesh.h"
#include "Renderer/RHI.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Types/SharedPtr.h"
#include <cstddef>
#include <vulkan/vulkan_core.h>

void FVkPipelineLayout::Initialize(const FVkPipelineLayoutCreateInfo& CreateInfo)
{
    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32>(CreateInfo.DescriptorSetLayouts.GetSize());
    PipelineLayoutCreateInfo.pSetLayouts = CreateInfo.DescriptorSetLayouts.GetData();

    VkResult Result = vkCreatePipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), &PipelineLayoutCreateInfo, nullptr, &Info.Handle);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create pipeline layout.");
}

//void PVulkanPipelineLayout::CreatePipelineLayout(FVulkanPipelineLayoutCreateInfo& CreateInfo)
//{
//    VkPushConstantRange PushConstantRange = {};
//	PushConstantRange.offset = 0;
//	PushConstantRange.size = 128;
//	PushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
//
//    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo{};
//    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    PipelineLayoutCreateInfo.pNext = nullptr;
//    PipelineLayoutCreateInfo.flags = 0;
//    PipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(CreateInfo.DescriptorSetLayouts.GetSize());
//    PipelineLayoutCreateInfo.pSetLayouts = CreateInfo.DescriptorSetLayouts.GetData();
//    PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
//    PipelineLayoutCreateInfo.pPushConstantRanges = &PushConstantRange;
//
//    VkResult Result = vkCreatePipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), &PipelineLayoutCreateInfo, nullptr, &Info.PipelineLayout);
//    RK_ASSERT(Result == VK_SUCCESS, "Failed to create pipeline layout.");
//}
//
//void PVulkanPipelineLayout::DestroyPipelineLayout()
//{
//    vkDestroyPipelineLayout(GetRHI()->GetDevice()->GetVkDevice(), Info.PipelineLayout, nullptr);
//}
//
//VkPipelineLayout PVulkanPipelineLayout::GetVkPipelineLayout() const
//{
//    return Info.PipelineLayout;
//}
//
//void PVulkanPipelineStateData::AddPipelineState(const FString& Name, const FVulkanPipelineStateCreateInfo& CreateInfo)
//{
//    std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;
//
//    for (PVulkanShader* Shader : CreateInfo.Stages)
//    {
//        VkPipelineShaderStageCreateInfo ShaderStageCreateInfo{};
//        ShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//        ShaderStageCreateInfo.pNext = nullptr;
//        ShaderStageCreateInfo.pName = "main";
//        ShaderStageCreateInfo.module = Shader->Info.ShaderModule;
//        ShaderStageCreateInfo.stage = Shader->Info.Stage;
//        
//        ShaderStageCreateInfos.push_back(ShaderStageCreateInfo);
//    }
//
//    VkFormat ColorAttachmentFormat = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetVkFormat();
//    std::vector<VkDynamicState> DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
//
//    VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo{};
//    InputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//    InputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
//
//    VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo{};
//    RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
//    RasterizationStateCreateInfo.lineWidth = 1.0f;
//    RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
//    RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
//
//    VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo{};
//    MultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
//    MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//    MultisampleStateCreateInfo.minSampleShading = 1.0f;
//    MultisampleStateCreateInfo.pSampleMask = nullptr;
//    MultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
//    MultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
//
//    VkPipelineColorBlendAttachmentState ColorBlendAttachmentState{};
//    ColorBlendAttachmentState.blendEnable = VK_TRUE;
//    ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//    ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//    ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
//    ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//    ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//    ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
//
//    VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo{};
//    DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    DepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
//    DepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
//    DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
//    DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
//    DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
//    DepthStencilStateCreateInfo.front = {};
//    DepthStencilStateCreateInfo.back = {};
//    DepthStencilStateCreateInfo.minDepthBounds = 0.0f;
//    DepthStencilStateCreateInfo.maxDepthBounds = 1.0f;
//
//    VkPipelineRenderingCreateInfo RenderingCreateInfo{};
//    RenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
//    RenderingCreateInfo.colorAttachmentCount = 1;
//    RenderingCreateInfo.pColorAttachmentFormats = &ColorAttachmentFormat;
//    RenderingCreateInfo.depthAttachmentFormat = GetRHI()->GetSceneRenderer()->GetDepthImage()->GetVkFormat();
//
//    VkPipelineViewportStateCreateInfo ViewportStateCreateInfo{};
//    ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    ViewportStateCreateInfo.pNext = nullptr;
//    ViewportStateCreateInfo.viewportCount = 1;
//    ViewportStateCreateInfo.scissorCount = 1;
//
//    VkPipelineColorBlendStateCreateInfo ColorBlendingCreateInfo{};
//    ColorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    ColorBlendingCreateInfo.pNext = nullptr;
//    ColorBlendingCreateInfo.logicOpEnable = VK_FALSE;
//    ColorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
//    ColorBlendingCreateInfo.attachmentCount = 1;
//    ColorBlendingCreateInfo.pAttachments = &ColorBlendAttachmentState;
//
//    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo{};
//    VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//
//    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo{};
//    DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//    DynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
//    DynamicStateCreateInfo.pDynamicStates = DynamicStates.data();
//
//    VkGraphicsPipelineCreateInfo PipelineCreateInfo{};
//    PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    PipelineCreateInfo.pNext = &RenderingCreateInfo;
//    PipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
//    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;
//    PipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
//    PipelineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
//    PipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
//    PipelineCreateInfo.pColorBlendState = &ColorBlendingCreateInfo;
//    PipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;
//    PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
//    PipelineCreateInfo.pStages = ShaderStageCreateInfos.data();
//    PipelineCreateInfo.stageCount = static_cast<uint32_t>(ShaderStageCreateInfos.size());
//    PipelineCreateInfo.layout = CreateInfo.PipelineLayout->GetVkPipelineLayout();
//
//    FVulkanPipelineState PipelineState;
//
//    VkResult Result = vkCreateGraphicsPipelines(GetRHI()->GetDevice()->GetVkDevice(), VK_NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &PipelineState.Handle);
//    RK_ASSERT(Result == VK_SUCCESS, "Failed to create graphics pipeline.");
//
//    PipelineStateCache.Insert(Name, PipelineState);
//}

void FVkPipeline::Initialize(FVkPipelineCreateInfo& CreateInfo)
{
    TArray<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;

    for (const TSharedPtr<PVulkanShader>& Shader : CreateInfo.Shaders)
    {
        VkPipelineShaderStageCreateInfo ShaderStageCreateInfo = {};
        ShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageCreateInfo.pNext = nullptr;
        ShaderStageCreateInfo.pName = "main";
        ShaderStageCreateInfo.module = Shader->Info.Module;
        ShaderStageCreateInfo.stage = Shader->Info.Stage;
        ShaderStageCreateInfos.Add(ShaderStageCreateInfo);
    }

    VkFormat ColorAttachmentFormat = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetVkFormat();
    TArray<VkDynamicState> DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkVertexInputBindingDescription VertexInputBindingDescription = {};
    VertexInputBindingDescription.binding = 0;
    VertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    VertexInputBindingDescription.stride = sizeof(SVertex);

    std::array<VkVertexInputAttributeDescription, 1> AttributeDescriptions = {};
    AttributeDescriptions.at(0).binding = 0;
    AttributeDescriptions.at(0).location = 0;
    AttributeDescriptions.at(0).format = VK_FORMAT_R32G32B32_SFLOAT;
    AttributeDescriptions.at(0).offset = offsetof(SVertex, Position);

    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = {};
    VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    VertexInputStateCreateInfo.pVertexBindingDescriptions = &VertexInputBindingDescription;
    VertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(AttributeDescriptions.size());
    VertexInputStateCreateInfo.pVertexAttributeDescriptions = AttributeDescriptions.data();

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
    ColorBlendAttachmentState.blendEnable = VK_TRUE;
    ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

    VkPipelineColorBlendStateCreateInfo ColorBlendingCreateInfo{};
    ColorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendingCreateInfo.pNext = nullptr;
    ColorBlendingCreateInfo.logicOpEnable = VK_FALSE;
    ColorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendingCreateInfo.attachmentCount = 1;
    ColorBlendingCreateInfo.pAttachments = &ColorBlendAttachmentState;

    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo{};
    DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(DynamicStates.GetSize());
    DynamicStateCreateInfo.pDynamicStates = DynamicStates.GetData();

    VkGraphicsPipelineCreateInfo PipelineCreateInfo{};
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
}