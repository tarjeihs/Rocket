#pragma once

#include <glm/glm.hpp>

struct FVertex
{
    FVertex()
    {
        static_assert(sizeof(FVertex) == 48, "FVertex size is required to be 48 bytes.");

        Position    = glm::vec3(0.0f);
        Normal      = glm::vec3(0.0f);
        TexCoord    = glm::vec2(0.0f);
    }

    alignas(16) glm::vec3 Position;
    alignas(16) glm::vec3 Normal;
    alignas(16) glm::vec2 TexCoord;
};

class IMesh
{
public:
    virtual ~IMesh() = default;
};
