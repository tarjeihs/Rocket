#pragma once

#include "Renderer/Common/Mesh.h"
#include "Types/Vertex.h"

struct FVkMeshInfo
{
    TArray<FVertex> Vertices;
    TArray<uint32> Indices;
    
    uint32 Group;
    uint32 Index;
};

class FVkMesh : public IMesh
{
public:
    FVkMeshInfo Info;

    virtual void CreateMesh(FGeometry& Geometry) override;
    virtual void UpdateMesh(FGeometry& Geometry) override;
};