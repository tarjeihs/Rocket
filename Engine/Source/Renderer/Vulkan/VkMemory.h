#pragma once

#include "Renderer/Common/Memory.h"
#include "Types/Map.h"
#include "Types/DoubleLinkedList.h"
#include "EngineTypes.h"

static constexpr uint32 VK_GLOBAL_BUFFER_INDEX = 0;
static constexpr uint32 VK_CAMERA_BUFFER_INDEX = 1;
static constexpr uint32 VK_MATERIAL_BUFFER_INDEX = 2;
static constexpr uint32 VK_INSTANCE_BUFFER_INDEX = 3;

static constexpr uint32 VK_INTERMEDIATE_COLOR_ATTACHMENT_IMAGE_INDEX = 0;
static constexpr uint32 VK_PRESENT_COLOR_ATTACHMENT_IMAGE_INDEX = 1;
static constexpr uint32 VK_DEPTH_COLOR_ATTACHMENT_IMAGE_INDEX = 2;
static constexpr uint32 VK_TONEMAPPING_HDR_IMAGE_INDEX = 3;
static constexpr uint32 VK_TONEMAPPING_SDR_IMAGE_INDEX = 4;

class FVkDescriptorPool;
class FVkDescriptorSet;
class FVkDescriptorSetLayout;
class FVkPipelineLayout;
class FVkBuffer;
class FVkImage;

struct FVkMemoryInfo
{
	struct FFrame
	{
		TMap<FString, FVkBuffer*> Buffers;
		TMap<FString, FVkImage*> Images;

		FVkDescriptorSet* DescriptorSet;
	} Frames [CONCURRENT_FRAME_COUNT];

	FVkDescriptorPool* DescriptorPool;
	FVkDescriptorSetLayout* DescriptorSetLayout;
	FVkPipelineLayout* PipelineLayout;
};

class FVkMemory : public IMemory
{
public:
	FVkMemoryInfo Info;

	virtual void Initialize() override;
	virtual void Shutdown() override;
	virtual void Execute() override;

	virtual void AddBuffer(const FString& Name, const FBufferCreateInfo& CreateInfo, uint32 Frame) override;
	virtual void RemoveBuffer(const FString& Name, uint32 Frame) override;
	virtual IBuffer* GetBuffer(const FString& Name, uint32 Frame) const override;
	virtual void BindBuffer(const FString& Name, uint32 Binding, uint32 Frame) override;

	virtual void AddImage(const FString& Name, const FImageCreateInfo& CreateInfo, uint32 Frame) override;
	virtual void RemoveImage(const FString& Name, uint32 Frame) override;
	virtual IImage* GetImage(const FString& Name, uint32 Frame) const override;
	virtual void BindImage(const FString& Name, uint32 Binding, uint32 Frame) override;
};