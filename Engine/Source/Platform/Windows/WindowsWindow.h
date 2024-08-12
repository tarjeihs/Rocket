#pragma once

#include "Core/Window.h"

class PWindowsWindow : public IWindow
{
public:
	PWindowsWindow(const SWindowSpecification& InWindowSpecification)
		: IWindow(InWindowSpecification)
	{
	}

	virtual void CreateNativeWindow() override;
	virtual void DestroyNativeWindow() override;
	virtual void Poll() override;
	virtual void Swap() override;
	virtual bool ShouldClose() const override;
	virtual bool IsMinimized() const override;
	virtual void SetIsMinimized(bool bMinimized) override;
};