#pragma once
#include "RHIQueue.h"

class IRHIDevice
{
public:
    virtual ~IRHIDevice() = default;
#define RK_PLATFORM_VULKAN
#ifdef RK_PLATFORM_VULKAN
    virtual void* GetNativeDevice() const { return nullptr; }
    virtual void* GetNativeDriver() const { return nullptr; }
    virtual void* GetNativeInstance() const { return nullptr; }
#endif

    virtual IRHIQueue* GetGraphicsQueue() { return nullptr; }
    virtual IRHIQueue* GetComputeQueue() { return nullptr; }
    virtual IRHIQueue* GetTransferQueue() { return nullptr; }
    virtual IRHIQueue* GetPresentQueue() { return nullptr; }

    virtual void WaitUntilIdle() const = 0;
};
