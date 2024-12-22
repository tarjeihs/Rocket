#include "EnginePCH.h"
#include "VulkanSampler.h"

#include "Renderer/Vulkan/VulkanDevice.h"

void FVkSampler::Initialize(FVkSamplerCreateInfo& CreateInfo)
{
    VkSamplerCreateInfo SamplerCreateInfo = {};
    SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;  // Magnification filter (e.g., linear filtering)
    SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;  // Minification filter (e.g., linear filtering)
    SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;  // Mipmap filtering
    SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;  // V axis (vertical)
    SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;  // W axis (depth or third dimension for cubemaps)}
    SamplerCreateInfo.anisotropyEnable = VK_TRUE;  // Enable anisotropic filtering
    SamplerCreateInfo.maxAnisotropy = 16.0f;       // Maximum anisotropy level (if supported by GPU)
    SamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;  // Border color for sampling outside [0,1]
    SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;  // Use normalized coordinates (true would be for pixel-based access)
    SamplerCreateInfo.compareEnable = VK_FALSE;  // Used for shadow maps (disabled here)
    SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;  // Comparison function (not relevant unless compareEnable is true)

    VkResult Result = vkCreateSampler(GetRHI()->GetDevice()->GetVkDevice(), &SamplerCreateInfo, VK_NULL_HANDLE, &Info.Handle);
    RK_ASSERT(Result == VK_SUCCESS, "Sampler creation failed.");
}

void FVkSampler::Shutdown()
{
    vkDestroySampler(GetRHI()->GetDevice()->GetVkDevice(), Info.Handle, nullptr);
}

void PVulkanSampler::CreateSampler()
{
    VkSamplerCreateInfo SamplerCreateInfo = {};
    SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;  // Magnification filter (e.g., linear filtering)
    SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;  // Minification filter (e.g., linear filtering)
    SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;  // Mipmap filtering
    SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;  // V axis (vertical)
    SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;  // W axis (depth or third dimension for cubemaps)}
    SamplerCreateInfo.anisotropyEnable = VK_TRUE;  // Enable anisotropic filtering
    SamplerCreateInfo.maxAnisotropy = 16.0f;       // Maximum anisotropy level (if supported by GPU)
    SamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;  // Border color for sampling outside [0,1]
    SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;  // Use normalized coordinates (true would be for pixel-based access)
    SamplerCreateInfo.compareEnable = VK_FALSE;  // Used for shadow maps (disabled here)
    SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;  // Comparison function (not relevant unless compareEnable is true)

    VkResult Result = vkCreateSampler(GetRHI()->GetDevice()->GetVkDevice(), &SamplerCreateInfo, nullptr, &Sampler);
    RK_ASSERT(Result == VK_SUCCESS, "Sampler creation failed.");
}

void PVulkanSampler::DestroySampler()
{
    vkDestroySampler(GetRHI()->GetDevice()->GetVkDevice(), Sampler, nullptr);
}

VkSampler PVulkanSampler::GetSampler() const
{
    return Sampler;
}