#pragma once

#include "Types/String.h"

enum class EShaderStage
{
	Vertex,
	Pixel,
	Compute
};

struct FShaderCreateInfo
{
	std::string Path;
	EShaderStage Stage;
};

class IShader
{
public:
	virtual ~IShader() = default;

	virtual void Init(FShaderCreateInfo& CreateInfo) = 0;
	virtual void Free() = 0;
};