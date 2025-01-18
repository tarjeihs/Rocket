#pragma once

#include "Types/Vertex.h"
#include "Renderer/Common/Geometry.h"

class IMesh
{
public:
    virtual ~IMesh() = default;

    virtual void CreateMesh(FSubmesh& Submesh) = 0;
    virtual void UpdateMesh(FSubmesh& Submesh) = 0;
};