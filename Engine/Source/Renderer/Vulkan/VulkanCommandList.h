#pragma once

#include "Renderer/Common/RHICommandList.h"

class CVulkanViewport;
class CVulkanDevice;

class CVulkanCommandList : public IRHICommandList
{
public:
    // IRHICommandList interface
    virtual void Enqueue(std::function<void()>&& Command) override;
    virtual void Begin() override;
    virtual void Execute() override;
    virtual void End() override;
    virtual void Reset() override;
    virtual void* GetNativeHandle() const override { return CommandBuffer; }
    virtual void Initialize() override;
    virtual void Shutdown() override;

    std::vector<std::function<void()>> CommandList;

    VkCommandPool CommandPool;
    VkCommandBuffer CommandBuffer;
};
