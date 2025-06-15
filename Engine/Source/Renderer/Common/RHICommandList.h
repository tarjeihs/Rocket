#pragma once

#include <functional>

class IRHICommandList
{
public:
    virtual ~IRHICommandList() = default;

    virtual void Enqueue(std::function<void()>&& Command) = 0;
    virtual void Execute() = 0;
    virtual void Reset() = 0;
};
