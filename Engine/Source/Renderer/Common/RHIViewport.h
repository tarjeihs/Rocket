#pragma once

class IRHICommandList;

class IRHIViewport
{
public:
    virtual ~IRHIViewport() = default;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;
    virtual void Resize() = 0;
};