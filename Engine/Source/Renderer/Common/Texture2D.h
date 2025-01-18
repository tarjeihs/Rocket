#pragma once

enum class ETexture2DFormat
{
    SRGB, UNORM
};

struct FTexture2DCreateInfo
{
    unsigned char* Data;
    uint32 Width = 0, Height = 0, Components = 0, Bits = 0;
    ETexture2DFormat Format;
};

class ITexture2D
{
public:
    ~ITexture2D() = default;

    virtual void Initialize(FTexture2DCreateInfo CreateInfo) = 0;
    virtual void Shutdown() = 0;

    virtual int32 GetTextureID() const = 0;
};