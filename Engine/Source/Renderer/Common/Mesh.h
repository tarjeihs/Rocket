#pragma once

#include "Types/Vertex.h"
#include "Renderer/Common/Geometry.h"

class IMesh
{
public:
    virtual ~IMesh() = default;

    virtual void CreateMesh(FGeometry& Geometry) = 0;
    virtual void UpdateMesh(FGeometry& Geometry) = 0;
};