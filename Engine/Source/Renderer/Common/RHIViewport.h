#pragma once

class IRHICommandList;

class IRHIViewport
{
public:
    virtual ~IRHIViewport() = default;

    virtual void Swap(IRHICommandList& CmdList) = 0;
    virtual void Present(IRHICommandList& CmdList) = 0;
};