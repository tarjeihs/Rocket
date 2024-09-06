#pragma once

#include "Renderer/Common/Overlay.h"

class PVulkanRHI;
class PVulkanCommandBuffer;
class PVulkanDescriptorPool;

class PVulkanOverlay : public POverlay
{
public:
	virtual void Init() override;
	virtual void Shutdown() override;

private:
	PVulkanDescriptorPool* DescriptorPool;
};