#pragma once

#include "Renderer/Common/Overlay.h"

class PVulkanRHI;
class PVulkanCommandBuffer;
class FVkDescriptorPool;

class PVulkanOverlay : public POverlay // TODO: Can this be seperated into it's own renderer? It probably should be 
{
public:
	virtual void Init() override;
	virtual void Shutdown() override;

private:
	FVkDescriptorPool* DescriptorPool;
};