#pragma once

#include "Renderer/Common/Image.h"
#include "Renderer/Common/Shader.h"
#include "Types/Array.h"
#include "Types/Map.h"
#include "Types/String.h"
#include "EngineTypes.h"

struct FPipelineCreateInfo
{
	TArray<FShaderCreateInfo> ShaderCreateInfos;
};

struct FPipelineRenderingAttachment
{
	FString Name;
};

struct FPipelineExecuteInfo
{
	TMap<uint32, FString> Buffers;
	TMap<uint32, FString> Images;
	
	FPipelineRenderingAttachment ColorAttachment;
	FPipelineRenderingAttachment DepthAttachment;
	FPipelineRenderingAttachment StencilAttachment;
};

struct IPipeline
{
	virtual void Initialize(FPipelineCreateInfo& CreateInfo) = 0;
	virtual void Execute(FPipelineExecuteInfo& ExecuteInfo) = 0;
};