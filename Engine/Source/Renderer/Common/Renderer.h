#pragma once

class IRenderer 
{
public:
    virtual ~IRenderer() = default;
};

class IRendererFrontend
{
public:
    virtual void Initialize(class IMemory* Memory) = 0;
};