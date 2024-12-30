#pragma once

enum class EImageFormat
{
    RGBA32_SRGB,
    RGBA32_UNORM  
};

struct FTexture2DCreateInfo
{
    FString Path;
    EImageFormat ImageFormat;
};

class ITexture2D
{
public:
    ~ITexture2D() = default;

    virtual void Initialize(FTexture2DCreateInfo CreateInfo) = 0;
    virtual void Shutdown() = 0;
};