#pragma once

#include <glm/glm.hpp>

struct FVertex
{
    FVertex()
    {
        Position    = glm::vec3(0.0f);
        Normal      = glm::vec3(0.0f);
        TexCoord    = glm::vec2(0.0f);
        Tangent     = glm::vec3(0.0f);
        Bitangent   = glm::vec3(0.0f);
    }

    FVertex(const glm::vec3& InPosition, const glm::vec3& InNormal, const glm::vec2& InTexCoord, const glm::vec3& InTangent, const glm::vec3& InBitangent)
        : Position(InPosition), Normal(InNormal), TexCoord(InTexCoord), Tangent(InBitangent), Bitangent(InBitangent)
    {
    }

    alignas(16) glm::vec3 Position;
    alignas(16) glm::vec3 Normal;
    alignas(16) glm::vec2 TexCoord;
    alignas(16) glm::vec3 Tangent;
    alignas(16) glm::vec3 Bitangent;
};