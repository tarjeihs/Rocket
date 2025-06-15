#pragma once

class IRHIDevice
{
public:
    virtual ~IRHIDevice() = default;

    virtual void* GetNativeInstance() const { return nullptr; }

    virtual void WaitUntilIdle() const = 0;
};