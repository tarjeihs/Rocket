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
	virtual bool ShouldClose() const override;
	virtual bool IsMinimized() const override;
	virtual bool IsFocused() const;
	virtual void SetIsMinimized(bool bMinimized) override;
	virtual void SetIsFocused(bool bFocused) override;
	virtual void WaitEventOrTimeout(float TimeoutSeconds) override;
};