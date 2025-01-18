#pragma once

#include "Renderer/Common/Texture2D.h"

class PVulkanImage;
class PVulkanSampler;
class FVkImage;
class FVkSampler;

struct FVkTexture2DInfo
{
    FVkImage* Image;
    FVkSampler* Sampler;
};

struct FVkTexture2D : public ITexture2D
{
    FVkTexture2DInfo Info;

    virtual void Initialize(FTexture2DCreateInfo CreateInfo) override;
    virtual void Shutdown() override;

    virtual int32 GetTextureID() const override;
};