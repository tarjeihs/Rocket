#include "EnginePCH.h"
#include "VulkanMaterial.h"

#include <cstring>

#include "VulkanMesh.h"

void PVulkanMaterial::Init()
{
    GraphicsPipeline = std::make_shared<PVulkanGraphicsPipeline>();
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

void PVulkanMaterial::Serialize(std::string_view Path)
{
    // Missing implementation
}

void PVulkanMaterial::Deserialize(std::string_view Path)
{
    // Missing implementation
}