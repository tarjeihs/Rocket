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

    void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::vec2 Value);
    void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::vec3 Value);
    void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::vec4 Value);
    void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat2 Value);
    void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat3 Value);
    void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat4 Value);

public:
    PVulkanGraphicsPipeline* GraphicsPipeline;
};
