#pragma once

struct FVkSamplerCreateInfo
{

};

struct FVkSamplerInfo
{
    VkSampler Handle;
};

struct FVkSampler
{
    FVkSamplerInfo Info;

    void Initialize(FVkSamplerCreateInfo& CreateInfo);
    void Shutdown();
};