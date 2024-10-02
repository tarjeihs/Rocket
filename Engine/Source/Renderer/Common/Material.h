#pragma once

#include "Asset/Asset.h"

class IMesh;
class PShader;

class IMaterial : public IAssetMarshalInterface
{
public:
    virtual ~IMaterial() = default;

    virtual void Init(PShader* VertexShader, PShader* FragmentShader) = 0;
    virtual void Destroy() = 0;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

protected:
    IMaterial() = default;
};
