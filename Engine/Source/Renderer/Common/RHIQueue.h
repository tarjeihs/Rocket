#pragma once

struct FRHIPayload;

enum class ERHIQueueClass
{
    Graphics,   // G+C+T
    Compute,    // C (+T)
    Transfer    // T only
};

struct FRHIQueueCapabilities
{
    /*
     * Present to a swapchain
     */
    bool Present = false;
    /*
     * Sparse / tiled resources
     */
    bool Sparse = false;
    /*
     * Video encode / decode, optical flow
     */
    bool Video = false;
    /*
     * HW execution engine index - only required with Vulkan and D3D12. Metal and WebGPU pushes this step down to the driver.
     */
    uint16_t Engine = 0;
};

class IRHIQueue
{
public:
    virtual ~IRHIQueue() = default;

    virtual void Submit(FRHIPayload* Payload) = 0;
    virtual void WaitIdle() = 0;
    virtual ERHIQueueClass GetClass() const = 0;
    virtual const FRHIQueueCapabilities& GetCapabilities() const = 0;
    virtual void* GetNativeHandle() const { return nullptr; }
};