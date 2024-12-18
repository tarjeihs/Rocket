#pragma once

class PVulkanSampler
{
public:
    void CreateSampler();
    void DestroySampler();

    VkSampler GetSampler() const;

private:
    VkSampler Sampler;
};