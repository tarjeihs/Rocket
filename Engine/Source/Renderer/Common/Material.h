#pragma once

class IMesh;
class IShader;

enum class ESurfaceType
{
    Opaque
};

struct SMaterialBinaryData
{
    ESurfaceType SurfaceType;
};

class IMaterial
{
public:
    virtual ~IMaterial() = default;

    virtual void CreateMaterial(const SMaterialBinaryData& MaterialData) = 0;
    virtual void Destroy() = 0;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual void SetShader(IShader* Shader) = 0;

    virtual void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, float Value) = 0;
    virtual void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::vec2 Value) = 0;
    virtual void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::vec3 Value) = 0;
    virtual void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::vec4 Value) = 0;
    virtual void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat2 Value) = 0;
    virtual void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat3 Value) = 0;
    virtual void SetUniformValue(const uint32_t Set, const std::string& UniformName, const std::string& MemberName, glm::mat4 Value) = 0;

protected:
    IMaterial() = default;
};
