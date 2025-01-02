#pragma once

#include "Renderer/Common/Overlay.h"

class PVulkanRHI;
class PVulkanCommandBuffer;
class FVkDescriptorPool;

struct FVkOverlayInfo
{
    FVkDescriptorPool* DescriptorPool;
};

class FVkOverlay : public FOverlay
{
public:
    FVkOverlayInfo Info;

	virtual void Init() override;
	virtual void Shutdown() override;
    virtual void Execute() override;
};