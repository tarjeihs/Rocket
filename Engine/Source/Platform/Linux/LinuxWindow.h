#pragma once

#include "Core/Window.h"

class PLinuxWindow : IWindow 
{
    PLinuxWindow(const SWindowSpecification& InWindowSpecification)
        : IWindow(InWindowSpecification)
	{
	}

    virtual void CreateNativeWindow() override;
	virtual void DestroyNativeWindow() override;
	virtual void Poll() override;
	virtual bool ShouldClose() const override;
	virtual bool IsMinimized() const override;
	virtual void SetIsMinimized(bool bMinimized) override;
};