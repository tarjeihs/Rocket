#pragma once

#include "IO/Public/Window.h"

#ifndef _GLFW_WAYLAND
	#define _GLFW_WAYLAND
#endif

class CGLFWWindow : public IWindow
{
public:
	CGLFWWindow(const SWindowSpecification& InWindowSpecification)
		: IWindow(InWindowSpecification)
	{
	}

	virtual void CreateNativeWindow() override;
	virtual void DestroyNativeWindow() override;
	virtual void Poll() override;
	virtual bool ShouldClose() const override;
	virtual bool IsMinimized() const override;
	virtual bool IsFocused() const override;
	virtual void SetIsMinimized(bool bMinimized) override;
	virtual void SetIsFocused(bool bFocused) override;
	virtual void SetFocus(bool bFocus);
	virtual void WaitEventOrTimeout(float TimeoutSeconds) override;
};
