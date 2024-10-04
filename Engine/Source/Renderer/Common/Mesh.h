#pragma once

#include <glm/glm.hpp>

#include "Renderer/Common/Material.h"

struct STransform;

struct SVertex
{
    SVertex()
    {
        Position = glm::vec3(0.0f);
        TexCoord = glm::vec2(0.0f);
        Normal = glm::vec3(0.0f);
        Color = glm::vec4(1.0f);
    }

    alignas(16) glm::vec3 Position;
    alignas(16) glm::vec2 TexCoord;
    alignas(16) glm::vec3 Normal;
    alignas(16) glm::vec4 Color;
    alignas(16) glm::vec3 Tangent;
    alignas(16) glm::vec3 Bitangent;
};

struct SMeshBinaryData
{
    std::vector<SVertex> Vertices;
    std::vector<uint32_t> Indices;
};

struct SMeshSettings
{
    IMaterial* Material;
};

class IMesh
{
public:
    virtual ~IMesh() = default;

    virtual void CreateMesh(const SMeshBinaryData& MeshBinaryObject) = 0;
    virtual void DrawIndirectInstanced(uint32_t ID) = 0;
    virtual void Destroy() = 0;
    
    virtual IMaterial* GetMaterial() const = 0;
    virtual void SetMaterial(IMaterial* NewMaterial) = 0;
};
