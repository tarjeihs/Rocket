#pragma once

class IRHIQueue;
class RHIPresentMode;

struct RHISwapchainDesc
{
    uint32_t           Width;
    uint32_t           Height;
    ERHIFormat         BackBufferFormat;
    ERHIPresentMode    PresentMode;
    IRHIQueue*         PresentQueue;
    IRHIQueue*         GraphicsQueue;
};

class IRHISwapchain
{
    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Resize() = 0;
};