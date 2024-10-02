#pragma once

#include "Renderer/Common/Texture2D.h"

class PVulkanImage;
class PVulkanSampler;

class PVulkanTexture2D : public ITexture2D
{
public:
    virtual void CreateTexture2D(unsigned char* Data) override;
    virtual void DestroyTexture2D() override;

    virtual void Deserialize(SBlob& Blob) override;
    virtual void Cleanup() override;

protected:
    int Width;
    int Height;
    int Channels;

    PVulkanImage* Image;
    PVulkanSampler* Sampler;
};