#pragma once

struct VkSampler_T;

typedef VkSampler_T* VkSampler;

class PVulkanSampler
{
public:
    void CreateSampler();
    void DestroySampler();

    VkSampler GetSampler() const;

private:
    VkSampler Sampler;
};