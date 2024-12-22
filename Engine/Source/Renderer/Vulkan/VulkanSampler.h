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

class PVulkanSampler
{
public:
    void CreateSampler();
    void DestroySampler();

    VkSampler GetSampler() const;

private:
    VkSampler Sampler;
};