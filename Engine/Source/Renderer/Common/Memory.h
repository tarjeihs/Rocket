#pragma once

#include "EngineTypes.h"
#include "Renderer/Common/Image.h"
#include "Renderer/Common/Buffer.h"

class ITexture2D;
class IBuffer;
class IPipeline2;

class IMemory
{
public:
	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual void Execute() = 0;

	virtual void AddBuffer(const FString& Name, const FBufferCreateInfo& CreateInfo, uint32 Frame) = 0;
	virtual void RemoveBuffer(const FString& Name, uint32 Frame) = 0;
	virtual IBuffer* GetBuffer(const FString& Name, uint32 Frame) const = 0;
	virtual void BindBuffer(const FString& Name, uint32 Binding, uint32 Frame) = 0;

	virtual void AddImage(const FString& Name, const FImageCreateInfo& CreateInfo, uint32 Frame) = 0;
	virtual void RemoveImage(const FString& Name, uint32 Frame) = 0;
	virtual IImage* GetImage(const FString& Name, uint32 Frame) const = 0;
	virtual void BindImage(const FString& Name, uint32 Binding, uint32 Frame) = 0;
};

extern inline IMemory* GMemory = nullptr;