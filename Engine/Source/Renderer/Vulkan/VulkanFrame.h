#pragma once

#include "Renderer/Common/RHIFrame.h"

class IRHICommandList;

struct FVulkanFrame : public IRHIFrame
{
    IRHISemaphore* ImageAvailableSemaphore;        // signalled by acquire
    IRHISemaphore* RenderFinishedSemaphore;        // signalled by queue
    IRHIFence* InFlightFence;                      // waited by CPU
    IRHICommandList* CommandList;                  // primary CB
    uint32_t ImageIndex = 0;
};

//class CVulkanFrame
//{
//public:
//    IRHICommandList* GetPrimaryCmdList() const { return CmdList; }
//    void             SetPrimaryCmdList(IRHICommandList* c) { CmdList = c; }
//
//    void Begin(IRHIViewport* Viewport, uint64_t FrameIndex);
//    void End  (IRHIViewport* Viewport, IRHIQueue* GraphicsQueue, IRHIQueue* PresentQueue);
//
//private:
//    VkFence Fence;
//    VkSemaphore WaitSignal;
//    VkSemaphore ReadySignal;
//    uint32 NextImageIndex;
//
//    IRHICommandList* CmdList;
//};