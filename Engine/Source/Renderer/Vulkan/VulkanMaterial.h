#pragma once

#include "Renderer/Common/Material.h"

class PVulkanGraphicsPipeline;
class PVulkanDescriptorSet;
class PVulkanShader;

class PVulkanMaterial : public IMaterial
{
public:
    virtual void Init(PShader* VertexShader, PShader* FragmentShader) override;
    virtual void Destroy() override;
    virtual void Bind() const override;
    virtual void Unbind() const override;

    void SetValue(const uint32_t Set, const std::string& Name, const void* Data, const size_t Size, const size_t Offset = 0);

    void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat4 Value);

public:
    // TODO: Per frame data goes directly into PVulkanDescriptorSet.
    struct SMaterialFrameData
    {
        std::vector<PVulkanDescriptorSet*> DescriptorSets;
    } MaterialFrameData[3];

    PVulkanGraphicsPipeline* GraphicsPipeline;
};
