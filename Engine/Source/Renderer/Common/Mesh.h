#pragma once

#include <glm/glm.hpp>

struct SVertex
{
    SVertex()
    {
        static_assert(sizeof(SVertex) == 16, "SVertex size should be 16 bytes.");

        Position = glm::vec3(0.0f);
    }

    alignas(16) glm::vec3 Position;
};

struct SMeshBinaryData
{
    std::vector<SVertex> Vertices;
    std::vector<uint32_t> Indices;
};

class IMesh
{
public:
    virtual ~IMesh() = default;
};
