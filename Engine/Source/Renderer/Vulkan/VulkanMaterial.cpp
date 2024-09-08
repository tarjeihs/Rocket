#include "EnginePCH.h"
#include "VulkanMaterial.h"

#include "Renderer/Vulkan/VulkanMesh.h"
#include "Renderer/Vulkan/VulkanPipeline.h"

void PVulkanMaterial::Init()
{
    GraphicsPipeline = new PVulkanGraphicsPipeline();
    GraphicsPipeline->CreatePipeline();
}

void PVulkanMaterial::Destroy()
{
    GraphicsPipeline->DestroyPipeline();
}

void PVulkanMaterial::Bind() const
{
    GraphicsPipeline->Bind();
}

void PVulkanMaterial::Unbind() const
{
    GraphicsPipeline->Unbind();
}