#pragma once

#include "EngineTypes.h"
#include <Renderer/Vulkan/VulkanDescriptor.h>

class ITexture2D;
class IBuffer;
class IPipeline2;

static constexpr uint32 VK_GLOBAL_BUFFER_INDEX								= 0;
static constexpr uint32 VK_CAMERA_BUFFER_INDEX								= 1;
static constexpr uint32 VK_MATERIAL_BUFFER_INDEX							= 2;
static constexpr uint32 VK_INSTANCE_BUFFER_INDEX							= 3;

static constexpr uint32 VK_INTERMEDIATE_COLOR_ATTACHMENT_IMAGE_INDEX		= 0;
static constexpr uint32 VK_PRESENT_COLOR_ATTACHMENT_IMAGE_INDEX				= 1;
static constexpr uint32 VK_DEPTH_COLOR_ATTACHMENT_IMAGE_INDEX				= 2;
static constexpr uint32 VK_TONEMAPPING_HDR_IMAGE_INDEX						= 3;
static constexpr uint32 VK_TONEMAPPING_SDR_IMAGE_INDEX						= 4;

struct FMemoryStats
{
	SizeType BufferAllocationSize = 0;
};
static FMemoryStats MemoryStats;

class IMemory
{
public:
	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;

	virtual void AddBuffer(FBufferCreateInfo& CreateInfo, const FString& Name, uint32 Frame) = 0;
	virtual void RemoveBuffer(const FString& Name, uint32 Frame) = 0;
	virtual IBuffer* GetBuffer(const FString& Name, uint32 Frame) const = 0;
	virtual void BindBuffer(const FString& Name, uint32 Binding, uint32 Frame) = 0;

	virtual void AddImage(FImageCreateInfo& CreateInfo, const FString& Name, uint32 Frame) = 0;
	virtual void RemoveImage(const FString& Name, uint32 Frame) = 0;
	virtual IImage* GetImage(const FString& Name, uint32 Frame) const = 0;
	virtual void BindImage(const FString& Name, uint32 Binding, uint32 Frame) = 0;
};

struct FVkMemoryInfo
{
	TMap<FString, TDoubleLinkedList<IBuffer*>>					Buffer;
	TMap<FString, TDoubleLinkedList<IImage*>>					Image;

	FVkDescriptorPool*					DescriptorPool;
	FVkDescriptorSet*					DescriptorSet;
	FVkDescriptorSetLayout*				DescriptorSetLayout;

	FVkPipelineLayout*					PipelineLayout;
};

class FVkMemory : public IMemory
{
public:
	FVkMemoryInfo Info;
	
	virtual void Initialize() override;
	virtual void Shutdown() override;

	virtual void AddBuffer(FBufferCreateInfo& CreateInfo, const FString& Name, uint32 Frame) override;
	virtual void RemoveBuffer(const FString& Name, uint32 Frame) override;
	virtual IBuffer* GetBuffer(const FString& Name, uint32 Frame) const override;
	virtual void BindBuffer(const FString& Name, uint32 Binding, uint32 Frame) override;

	virtual void AddImage(FImageCreateInfo& CreateInfo, const FString& Name, uint32 Frame) override;
	virtual void RemoveImage(const FString& Name, uint32 Frame) override;
	virtual IImage* GetImage(const FString& Name, uint32 Frame) const override;
	virtual void BindImage(const FString& Name, uint32 Binding, uint32 Frame) override;
};

extern inline IMemory* GMemory = nullptr;