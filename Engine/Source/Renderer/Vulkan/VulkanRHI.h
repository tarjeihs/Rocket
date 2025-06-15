#pragma once

#include "Renderer/Common/RHI.h"

class CVulkanDevice;
class CVulkanViewport;
class CVulkanCommandList;

class CVulkanRHI final : public IRHI
{
public:
    // IRenderer interface
    virtual void Init() final override;
    virtual void Shutdown() final override;
    virtual void Resize() final override;
    virtual void Render() final override;

    virtual IRHIDevice* GetDevice() const override;
    virtual IRHIViewport* GetViewport() const override;
    virtual IRHICommandList* GetCommandList() const override;

private:
    TUniquePtr<CVulkanDevice> Device;
    TUniquePtr<CVulkanViewport> Viewport;
    TUniquePtr<CVulkanCommandList> CommandList;
};