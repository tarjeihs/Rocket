#pragma once

#include <cstdint>

#include "Core/Engine.h"

struct SWindowSpecification
{
	const char* Name;

	uint32_t Width;
	
	uint32_t Height;
};

struct SWindowUserData
{
};

class IWindow
{
public:
	IWindow(const SWindowSpecification& InWindowSpecification)
		: WindowSpecification(InWindowSpecification)
	{
	}

	virtual ~IWindow() = default;

	virtual void CreateNativeWindow() = 0;
	virtual void DestroyNativeWindow() = 0;

	virtual void Poll() = 0;

	virtual bool ShouldClose() const = 0;
	virtual bool IsMinimized() const = 0;
	virtual void SetIsMinimized(bool bMinimized) = 0;

	inline void* GetNativeWindow() const;
	inline SWindowSpecification& GetWindowSpecification();
	inline SWindowUserData& GetWindowUserData();
	
	inline float GetAspectRatio() const;

protected:
	SWindowSpecification WindowSpecification;

	SWindowUserData WindowUserData;

	void* NativeWindow = nullptr;

	bool bIsMinimized = false;
};

inline void* IWindow::GetNativeWindow() const
{
	return NativeWindow;
}

inline SWindowSpecification& IWindow::GetWindowSpecification()
{
	return WindowSpecification;
}

inline SWindowUserData& IWindow::GetWindowUserData()
{
	return WindowUserData;
}

inline float IWindow::GetAspectRatio() const
{
	return static_cast<float>(WindowSpecification.Width) / static_cast<float>(WindowSpecification.Height);
}

static inline IWindow* GetWindow()
{
	return PEngine::Get()->GetWindow();
}