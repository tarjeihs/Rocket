#pragma once

#include <memory>

#include "VulkanPipeline.h"
#include "Renderer/Common/Material.h"

struct SVulkanBuffer;

class PVulkanMaterial : public IMaterial
{
public:
    virtual void Init() override;
    virtual void Destroy() override;
    virtual void Bind(STransform Transform) const override;
    virtual void Unbind() const override;

    virtual void Serialize(std::string_view Path) override;
    virtual void Deserialize(std::string_view Path) override;

public:
    std::shared_ptr<PVulkanGraphicsPipeline> GraphicsPipeline;
};
