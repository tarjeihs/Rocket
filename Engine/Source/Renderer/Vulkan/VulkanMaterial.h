#pragma once

#include "Renderer/Common/Material.h"

class PVulkanGraphicsPipeline;
struct PVulkanBuffer;

class PVulkanMaterial : public IMaterial
{
public:
    virtual void Init() override;
    virtual void Destroy() override;
    virtual void Bind() const override;
    virtual void Unbind() const override;

public:
    PVulkanGraphicsPipeline* GraphicsPipeline;
};
