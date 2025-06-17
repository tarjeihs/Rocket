#pragma once

#include "Renderer/Common/RHI.h"

class CVulkanFrame;
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

    virtual IRHIDevice* GetDevice() override;
    virtual IRHIViewport* GetViewport(uint32_t Index = 0) override;

private:
    TUniquePtr<CVulkanDevice> Device;
    std::vector<TUniquePtr<CVulkanViewport>> Viewports;
};