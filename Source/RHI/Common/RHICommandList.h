#pragma once

#include <functional>

/*
 * A transient per-frame object responsible for recording GPU-specific commands for deferred execution on a queue.
 */
class IRHICommandList
{
public:
    virtual ~IRHICommandList() = default;

    virtual void Enqueue(std::function<void()>&& Command) = 0;
    virtual void Begin() = 0;
    virtual void Execute() = 0;
    virtual void End() = 0;
    virtual void Reset() = 0;
    virtual void* GetNativeHandle() const { return nullptr; }
    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;
};
