#pragma once

#include "Renderer/Common/Mesh.h"

struct FVkMeshInfo
{
    FSubmesh Submeshes;
};

class FVkMesh : public IMesh
{
public:
    FVkMeshInfo Info;

    virtual void CreateMesh(FSubmesh& Submesh) override;
    virtual void UpdateMesh(FSubmesh& Submesh) override;
};