#pragma once

#include "Renderer/Common/Material.h"

class FVulkanPipeline;
class FVkDescriptorSet;
class FVkShader;

class PVulkanMaterial : public IMaterial
{
public:

public:
    FVulkanPipeline* GraphicsPipeline;
};
