#pragma once

#include <cstdint>

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
	virtual void Swap() = 0;

	virtual bool ShouldClose() const = 0;

	inline void* GetNativeWindow() const;
	inline const char* GetName() const;
	inline uint32_t GetWidth() const;
	inline uint32_t GetHeight() const;

protected:
	SWindowSpecification WindowSpecification;

	SWindowUserData WindowUserData;

	void* NativeWindow = nullptr;
};

inline void* IWindow::GetNativeWindow() const
{
	return NativeWindow;
}

inline const char* IWindow::GetName() const
{
	return WindowSpecification.Name;
}

inline uint32_t IWindow::GetWidth() const
{
	return WindowSpecification.Width;
}

inline uint32_t IWindow::GetHeight() const
{
	return WindowSpecification.Height;
}