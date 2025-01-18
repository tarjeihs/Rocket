#pragma once

#include "EngineTypes.h"

enum class EImageLayout
{
	None = 0,
	Undefined,
	General,
	ReadOnly
};

enum class EImageFormat
{
	None = 0,
	R16G16B16A16_SFLOAT,
	R16G16B16A16_UNORM,
	R16G16B16_SFLOAT,
	R16G16B16_UNORM,
	R16G16_SFLOAT,
	R16G16_UNORM,
	R16_SFLOAT,
	R16_UNORM,
	R8G8B8A8_SRGB,
	R8G8B8A8_UNORM,
	R8G8B8_SRGB,
	R8G8B8_UNORM,
	R8G8_SRGB,
	R8G8_UNORM,
	R8_SRGB,
	R8_UNORM,
	D32_SFLOAT,
	A2B10G10R10_UNORM_PACK32,
	A2R10G10B10_UNORM_PACK32
};

enum class EImageUsage : int8
{
	None					= 0,
	TransferSrc				= 1 << 0,
	TransferDst				= 1 << 1,
	Sampled					= 1 << 2,
	Storage					= 1 << 3,
	ColorAttachment			= 1 << 4,
	DepthStencilAttachment	= 1 << 5
};

enum class EImageAspect : int8
{
	None = 0,
	Color = 1 << 0,
	Depth = 1 << 1,
	Stencil = 1 << 2,
};

inline EImageUsage operator|(EImageUsage LHS, EImageUsage RHS) { return static_cast<EImageUsage>(static_cast<int8>(LHS) | static_cast<int8>(RHS)); }
inline EImageUsage operator&(EImageUsage LHS, EImageUsage RHS) { return static_cast<EImageUsage>(static_cast<int8>(LHS) & static_cast<int8>(RHS)); }
inline EImageAspect operator|(EImageAspect LHS, EImageAspect RHS) { return static_cast<EImageAspect>(static_cast<int8>(LHS) | static_cast<int8>(RHS)); }
inline EImageAspect operator&(EImageAspect LHS, EImageAspect RHS) { return static_cast<EImageAspect>(static_cast<int8>(LHS) & static_cast<int8>(RHS)); }

inline bool HasFlag(EImageUsage LHS, EImageUsage RHS) { return (static_cast<int8>(LHS) & static_cast<int8>(RHS)) != 0; }
inline bool HasFlag(EImageAspect LHS, EImageAspect RHS) { return (static_cast<int8>(LHS) & static_cast<int8>(RHS)) != 0; }

struct FImageExtent
{
	uint32 Width;
	uint32 Height;
};

struct FImageCreateInfo
{
	EImageLayout Layout;
	EImageUsage UsageFlags;
	EImageAspect AspectFlags;
	FImageExtent Extent;
	EImageFormat Format;
};

struct IImage
{
	virtual void Initialize(FImageCreateInfo& CreateInfo) = 0;
	virtual void Shutdown() = 0;
};