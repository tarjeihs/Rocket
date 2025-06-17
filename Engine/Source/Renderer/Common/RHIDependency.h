#pragma once

struct IRHIDependency
{
    virtual ~IRHIDependency() = default;

    virtual void* GetNativeHandle() const { return nullptr; }
    virtual void Invalidate() {}
};

struct IRHISemaphore : IRHIDependency {};
struct IRHIFence : IRHIDependency {};