#pragma once

class IMesh;
class IShader;

enum class ESurfaceType
{
    Opaque
};

struct SMaterialBinaryData
{
    ESurfaceType SurfaceType;
};

class IMaterial
{
public:
    virtual ~IMaterial() = default;

protected:
    IMaterial() = default;
};
