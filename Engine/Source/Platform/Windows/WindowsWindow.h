#pragma once

#include "Core/Window.h"

class PWindowsWindow : public IWindow
{
public:
	PWindowsWindow(const SWindowSpecification& InWindowSpecification)
		: IWindow(InWindowSpecification)
	{
	}

	void CreateNativeWindow() override;
	void DestroyNativeWindow() override;
	void Poll() override;
	void Swap() override;
	bool ShouldClose() const override;
};