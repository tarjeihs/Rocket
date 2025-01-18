#include "VkRendererFrontend.h"

#include "Renderer/Common/Memory.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"

FBufferCreateInfo GlobalBufferCreateInfo
{
	EBufferUsageFlag::Storage,
	EBufferTransferFlag::None,
	EBufferMemoryFlag::HostToDevice,
	64
};

FBufferCreateInfo CameraBufferCreateInfo
{
	EBufferUsageFlag::Storage,
	EBufferTransferFlag::None,
	EBufferMemoryFlag::HostToDevice,
	160 * 4
};

FBufferCreateInfo MaterialBufferCreateInfo
{
	EBufferUsageFlag::Storage,
	EBufferTransferFlag::None,
	EBufferMemoryFlag::HostToDevice,
	64 * 1024
};

FBufferCreateInfo InstanceBufferCreateInfo
{
	EBufferUsageFlag::Storage,
	EBufferTransferFlag::None,
	EBufferMemoryFlag::HostToDevice,
	128 * 1024 * 1024
};

//FImageCreateInfo ToneMappingInputCreateInfo =
//{
//	VK_IMAGE_LAYOUT_UNDEFINED,
//	VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
//	VK_IMAGE_ASPECT_COLOR_BIT
//};
//
//FImageCreateInfo ToneMappingOutputCreateInfo =
//{
//	VK_IMAGE_LAYOUT_UNDEFINED,
//	VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
//	VK_IMAGE_ASPECT_COLOR_BIT
//};

FImageCreateInfo ToneMappingInputCreateInfo =
{
	EImageLayout::Undefined,
	EImageUsage::Storage | EImageUsage::TransferDst | EImageUsage::TransferSrc,
	EImageAspect::Color
};

FImageCreateInfo ToneMappingOutputCreateInfo =
{
	EImageLayout::Undefined,
	EImageUsage::Storage | EImageUsage::TransferSrc,
	EImageAspect::Color
};

void FVkRendererFrontend::Initialize(IMemory* Memory)
{
	ToneMappingInputCreateInfo.Extent = {1280,720};
	ToneMappingInputCreateInfo.Format = EImageFormat::R16G16B16A16_SFLOAT;
	ToneMappingOutputCreateInfo.Extent = {1280,720};
	ToneMappingOutputCreateInfo.Format = EImageFormat::R16G16B16A16_SFLOAT;

	Memory->AddBuffer(GlobalBufferCreateInfo, "Global", CONCURRENT_FRAME_COUNT);
	//Memory->AddBuffer(CameraBufferCreateInfo, "Camera", CONCURRENT_FRAME_COUNT);
	//Memory->AddBuffer(MaterialBufferCreateInfo, "Material", CONCURRENT_FRAME_COUNT);
	//Memory->AddBuffer(InstanceBufferCreateInfo, "Instance", CONCURRENT_FRAME_COUNT);
	//
	//Memory->AddImage(ToneMappingInputCreateInfo, 0, FrameIdx);
	//Memory->AddImage(ToneMappingOutputCreateInfo, 1, FrameIdx);
	//Memory->WriteBuffer("Global", 0, GetFrameIndex());
}