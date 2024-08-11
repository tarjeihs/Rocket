#pragma once

#include "Renderer/RenderHardwareInterface.h"

class PVulkanRenderHardwareInterface : public IRenderHardwareInterface
{
public:
	virtual void CreateRHI() override;
	virtual void DestroyRHI() override;
	virtual void Render() override;
};