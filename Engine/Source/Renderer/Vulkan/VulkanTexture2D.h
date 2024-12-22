#pragma once

#include "Renderer/Common/Texture2D.h"

class PVulkanImage;
class PVulkanSampler;
class FVkImage;
class FVkSampler;

enum class EImageFormat
{
    
};

struct FVkTexture2DCreateInfo
{
    FString Path;
    VkFormat ImageFormat;
};

struct FVkTexture2DInfo
{
    FVkImage* Image;
    FVkSampler* Sampler;

    uint32 Width;
    uint32 Height;
    uint32 Channels;

    unsigned char* Data;
};

struct FVkTexture2D
{
    FVkTexture2DInfo Info;

    void Initialize(FVkTexture2DCreateInfo& CreateInfo);
    void Shutdown();
};