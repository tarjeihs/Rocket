#pragma once

#include "Renderer/Common/RHIQueue.h"

struct FRHIPayload;

class CVulkanQueue : public IRHIQueue
{
public:
    virtual void Submit(FRHIPayload* Payload) override;
    virtual void WaitIdle() override;
    virtual ERHIQueueClass GetClass() const override;
    virtual const FRHIQueueCapabilities& GetCapabilities() const override;
    virtual void* GetNativeHandle() const override;

private:
    ERHIQueueClass Class;
    FRHIQueueCapabilities Capabilities;

    VkQueue Queue;

    friend class CVulkanDevice;
};