#include "EnginePCH.h"
#include "VulkanPipeline.h"

#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanMemory.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanCommand.h"

void PVulkanPipelineLayout::CreatePipelineLayout(const std::vector<PVulkanDescriptorSetLayout*> DescriptorSetLayouts, const std::vector<VkPushConstantRange>& PushConstantRanges)
{
    std::vector<VkDescriptorSetLayout> DescriptorSetLayoutData;
    for (PVulkanDescriptorSetLayout* DescriptorSetLayout : DescriptorSetLayouts)
    {
        DescriptorSetLayoutData.push_back(DescriptorSetLayout->GetVkDescriptorSetLayout());
    }

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo{};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.pNext = nullptr;
    PipelineLayoutCreateInfo.flags = 0;
    PipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(DescriptorSetLayoutData.size());
    PipelineLayoutCreateInfo.pSetLayouts = DescriptorSetLayoutData.data();
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

void PVulkanGraphicsPipeline::CreatePipeline(PVulkanShader* Shader)
{
    std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;
    std::vector<PVulkanDescriptorSetLayout*> DescriptorSetLayouts;

    for (const SShaderModule& ShaderModule : Shader->GetShaderModules()) 
    {
        VkPipelineShaderStageCreateInfo VertexShaderStageCreateInfo{};
        VertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        VertexShaderStageCreateInfo.pNext = nullptr;
        VertexShaderStageCreateInfo.pName = "main";
        VertexShaderStageCreateInfo.module = ShaderModule.ShaderModule;
        VertexShaderStageCreateInfo.stage = ShaderModule.Flag;
        
        ShaderStageCreateInfos.push_back(VertexShaderStageCreateInfo);
        
        DescriptorSetLayouts.resize(DescriptorSetLayouts.size() + ShaderModule.DescriptorSetLayouts.size());
        memcpy(DescriptorSetLayouts.data() + DescriptorSetLayouts.size() - ShaderModule.DescriptorSetLayouts.size(), ShaderModule.DescriptorSetLayouts.data(), ShaderModule.DescriptorSetLayouts.size() * sizeof(PVulkanDescriptorSetLayout*));
    }

    VkPushConstantRange UInt64PointerPushConstantRange{};
    UInt64PointerPushConstantRange.offset = 0;
    UInt64PointerPushConstantRange.size = sizeof(SUInt64PointerPushConstant);
    UInt64PointerPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    PipelineLayout = new PVulkanPipelineLayout(); 
    PipelineLayout->CreatePipelineLayout(DescriptorSetLayouts, { UInt64PointerPushConstantRange });

    VkFormat ColorAttachmentFormat = GetRHI()->GetSceneRenderer()->GetDrawImage()->GetVkFormat();
    std::vector<VkDynamicState> DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

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
    PipelineLayout->DestroyPipelineLayout();
    vkDestroyPipeline(GetRHI()->GetDevice()->GetVkDevice(), Pipeline, nullptr);

    delete PipelineLayout;
}

void PVulkanGraphicsPipeline::Bind(std::vector<VkDescriptorSet> DescriptorSetData)
{
    PVulkanFrame* Frame = GetRHI()->GetSceneRenderer()->GetParallelFramePool()->GetCurrentFrame();

    vkCmdBindPipeline(Frame->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
    vkCmdBindDescriptorSets(Frame->GetCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout->GetVkPipelineLayout(), 0, DescriptorSetData.size(), DescriptorSetData.data(), 0, nullptr);
}

void PVulkanGraphicsPipeline::Unbind()
{
    
}

PVulkanPipelineLayout * PVulkanGraphicsPipeline::GetPipelineLayout() const
{
    return PipelineLayout;
}

VkPipeline PVulkanGraphicsPipeline::GetVkPipeline() const
{
    return Pipeline;
}
