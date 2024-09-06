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

void PVulkanMaterial::Bind(STransform Transform) const
{
    GraphicsPipeline->Bind(Transform);
}

void PVulkanMaterial::Unbind() const
{
    GraphicsPipeline->Unbind();
}