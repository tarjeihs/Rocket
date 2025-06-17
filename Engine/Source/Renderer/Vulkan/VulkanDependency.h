#pragma once

#include "VulkanRHI.h"
#include "Renderer/Common/RHIDependency.h"
#include "Renderer/Common/RHIDevice.h"

struct FVulkanSemaphore : IRHISemaphore
{
    virtual void* GetNativeHandle() const override { return Handle; }
    virtual void Invalidate() override
    {
        if (Handle != VK_NULL_HANDLE)
        {
            vkDestroySemaphore((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Handle, nullptr);
        }

        VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
        SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        SemaphoreCreateInfo.pNext = VK_NULL_HANDLE;
        SemaphoreCreateInfo.flags = 0;

        vkCreateSemaphore((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), &SemaphoreCreateInfo, nullptr, &Handle);
    }

    VkSemaphore Handle = VK_NULL_HANDLE;
};

struct FVulkanFence : IRHIFence
{
    virtual void* GetNativeHandle() const override { return Handle; }
    virtual void Invalidate() override
    {
        if (Handle != VK_NULL_HANDLE)
        {
            vkDestroyFence((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), Handle, nullptr);
        }
        VkFenceCreateInfo FenceCreateInfo = {};
        FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        FenceCreateInfo.pNext = VK_NULL_HANDLE;
        FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence((VkDevice)GetRHI()->GetDevice()->GetNativeDriver(), &FenceCreateInfo, nullptr, &Handle);
    }

    VkFence Handle = VK_NULL_HANDLE;
};
