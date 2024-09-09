#pragma once

#include <span>
#include <glm/glm.hpp>

#include "Asset/Asset.h"

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

    glm::vec3 Position;
    glm::vec2 TexCoord;
    glm::vec3 Normal;
    glm::vec4 Color;
};

struct SMeshStats
{
    uint32_t Vertices;
    uint32_t Triangles;
};

class IMesh : public IAssetMarshalInterface
{
public:
    virtual ~IMesh() = default;

    virtual void CreateMesh(std::span<SVertex> Vertices, std::span<uint32_t> Indices) = 0;
    virtual void DrawIndirectInstanced(uint32_t ID) = 0;
    virtual void Destroy() = 0;
    virtual void ApplyMaterial(class IMaterial* NewMaterial) = 0;

    SMeshStats Stats;
};
