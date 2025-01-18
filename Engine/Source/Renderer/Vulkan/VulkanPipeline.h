#pragma once

class FVkDescriptorSet;
class FVkDescriptorSetLayout;
class FVkShader;
class PVulkanImage;
class FVkBuffer;
class IPipeline;

struct FVkPipelineLayoutCreateInfo
{
	TArray<FVkDescriptorSetLayout*> DescriptorSetLayouts;
};

struct FVkPipelineLayoutInfo
{
	VkPipelineLayout Handle;
};

class FVkPipelineLayout
{
public:
	FVkPipelineLayoutInfo Info;

	void Initialize(const FVkPipelineLayoutCreateInfo& CreateInfo);
	void Shutdown();
};

struct FVkVertexAttribute
{
	uint32 Binding;
	uint32 Location;
	VkFormat Format;
	SizeType Offset;
	SizeType Stride;
};

struct FVkPipelineCreateInfo
{
	TArray<FVkShader*> Shaders;
	TArray<FVkVertexAttribute> Attributes;

	FVkPipelineLayout* PipelineLayout;
};

struct FVkPipelineInfo
{
	VkPipeline Handle;

	IPipeline* Pipeline;
};

struct FVkPipelineBeginInfo
{
	VkRenderingAttachmentInfo* ColorAttachment;
	VkRenderingAttachmentInfo* DepthAttachment;
	VkRenderingAttachmentInfo* StencilAttachment;
};

struct FVkPipelineEndInfo
{
};

class FVkPipeline
{
public:
	FVkPipelineInfo Info;

	virtual ~FVkPipeline() = default;

	virtual void Initialize(FVkPipelineCreateInfo& CreateInfo) = 0;
	virtual void Begin(FVkPipelineBeginInfo& BeginInfo) = 0;
	virtual void End(FVkPipelineEndInfo& EndInfo) = 0;
	
	void Shutdown();
};

class FVkPipelineGfx : public FVkPipeline
{
public:
	virtual void Initialize(FVkPipelineCreateInfo& CreateInfo) override;
	virtual void Begin(FVkPipelineBeginInfo& BeginInfo) override;
	virtual void End(FVkPipelineEndInfo& EndInfo) override;
};

class FVkPipelineCompute : public FVkPipeline
{
public:
	virtual void Initialize(FVkPipelineCreateInfo& CreateInfo) override;
	virtual void Begin(FVkPipelineBeginInfo& BeginInfo) override;
	virtual void End(FVkPipelineEndInfo& EndInfo) override;
};