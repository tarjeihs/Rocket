#pragma once

class IRHIFence;
class IRHISemaphore;
class IRHICommandList;

struct FRHIPayload
{
    FRHIPayload(IRHICommandList* InCommandList, IRHISemaphore* InImageAvailableSemaphore, IRHISemaphore* InRenderFinishedSemaphore, IRHIFence* InInFlightFence)
        : CommandList(InCommandList),
        ImageAvailableSemaphore(InImageAvailableSemaphore),
        RenderFinishedSemaphore(InRenderFinishedSemaphore),
        InFlightFence(InInFlightFence)
    {
    }

    inline IRHICommandList* GetCommandList() const { return CommandList; }
    inline IRHISemaphore* GetImageAvailableSemaphore() const { return ImageAvailableSemaphore; }
    inline IRHISemaphore* GetRenderFinishedSemaphore() const { return RenderFinishedSemaphore; }
    inline IRHIFence* GetInFlightFence() const { return InFlightFence; }

private:
    IRHICommandList* CommandList = nullptr;
    IRHISemaphore* ImageAvailableSemaphore = nullptr;
    IRHISemaphore* RenderFinishedSemaphore = nullptr;
    IRHIFence* InFlightFence = nullptr;
};