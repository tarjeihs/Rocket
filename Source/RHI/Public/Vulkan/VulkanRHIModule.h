#pragma once

#include "RHI/Public/Common/RHI.h"

class FVulkanRHIModule : public IRHIModule
{
public:
    virtual IRHI* CreateRHI() override;
};