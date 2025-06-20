#pragma once

#include "VulkanRHIMinimal.h"

class CVulkanRHI : public IVulkanRHIMinimal
{
    virtual void Init() override {}
    virtual void Shutdown() override {}
    virtual void Resize() override {}
    virtual void Render() override {}
};
