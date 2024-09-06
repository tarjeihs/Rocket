#pragma once

#include "Asset/Asset.h"
#include "Math/Transform.h"

class IMesh;

class IMaterial : public IAssetMarshalInterface
{
public:
    virtual ~IMaterial() = default;

    virtual void Init() = 0;
    virtual void Destroy() = 0;
    virtual void Bind(STransform Transform) const = 0;
    virtual void Unbind() const = 0;

protected:
    IMaterial() = default;
};
