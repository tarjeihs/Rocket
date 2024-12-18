#pragma once

#include "EngineTypes.h"
#include "Renderer/Common/Renderer.h"
#include "Renderer/Settings.h"
#include "Types/UniquePtr.h"

typedef uint64 FUUID;

class IRenderer;
class PVulkanAllocator;
class PVulkanSwapchain;
class PVulkanCommandPool;
class PVulkanCommandBuffer;
class PVulkanRenderGraph;
class FVkImage;
class FVkBuffer;
class FVkPipeline;
class FVkPipelineLayout;
class FVkDescriptorSet;
class FVkDescriptorSetLayout;

struct FRendererContext
{
    TUniquePtr<PVulkanCommandPool>              CommandPool             [CONCURRENT_FRAME_COUNT];
    TUniquePtr<PVulkanCommandBuffer>            CommandBuffer           [CONCURRENT_FRAME_COUNT];
    VkSemaphore                                 SwapchainSemaphore      [CONCURRENT_FRAME_COUNT];
    VkSemaphore                                 RenderSemaphore         [CONCURRENT_FRAME_COUNT];
    VkFence                                     RenderFence             [CONCURRENT_FRAME_COUNT];
    uint32                                      NextImageIndex          [CONCURRENT_FRAME_COUNT];
};

class FVulkanRenderer : public IRenderer
{
public:
    using Super = FVulkanRenderer;

    virtual void Init();
    virtual void Shutdown();
    void Render();
    void Resize();

    virtual void Bind() = 0;

    inline FVkImage* GetColorAttachmentImage() const;
    inline FVkImage* GetDepthAttachmentImage() const;
    inline PVulkanCommandPool* GetCommandPool() const;
    inline PVulkanCommandBuffer* GetCommandBuffer() const;
    inline PVulkanRenderGraph* GetRenderGraph() const;
    inline PVulkanSwapchain* GetSwapchain() const;
    inline SizeType GetFrameIndex() const;

    void ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func);

protected:
    void BeginFrame();
    void EndFrame();

protected: 
    TUniquePtr<PVulkanCommandPool>              CommandPool                     [CONCURRENT_FRAME_COUNT];
    TUniquePtr<PVulkanCommandBuffer>            CommandBuffer                   [CONCURRENT_FRAME_COUNT];
    VkSemaphore                                 SwapchainSemaphore              [CONCURRENT_FRAME_COUNT];
    VkSemaphore                                 RenderSemaphore                 [CONCURRENT_FRAME_COUNT];
    VkFence                                     RenderFence                     [CONCURRENT_FRAME_COUNT];
    uint32                                      NextImageIndex                  [CONCURRENT_FRAME_COUNT];

    TUniquePtr<PVulkanCommandBuffer>            ImmediateCommandBuffer;
    TUniquePtr<PVulkanCommandPool>              ImmediateCommandPool;
    VkFence                                     ImmediateRenderFence;

    TUniquePtr<PVulkanSwapchain>                Swapchain;
    TUniquePtr<PVulkanRenderGraph>              RenderGraph;

    TUniquePtr<FVkImage>                        ColorAttachmentImage;
    TUniquePtr<FVkImage>                        DepthAttachmentImage;

    SizeType                                    FrameIndex;
};

inline FVkImage* FVulkanRenderer::GetColorAttachmentImage() const
{
    return ColorAttachmentImage.Get();
}

inline FVkImage* FVulkanRenderer::GetDepthAttachmentImage() const
{
    return DepthAttachmentImage.Get();
}

inline PVulkanCommandPool* FVulkanRenderer::GetCommandPool() const
{
    return CommandPool[FrameIndex].Get();
}

inline PVulkanCommandBuffer* FVulkanRenderer::GetCommandBuffer() const
{
    return CommandBuffer[FrameIndex].Get();
}

inline PVulkanRenderGraph* FVulkanRenderer::GetRenderGraph() const
{
    return RenderGraph.Get();
}

inline PVulkanSwapchain* FVulkanRenderer::GetSwapchain() const
{
    return Swapchain.Get();
}

inline SizeType FVulkanRenderer::GetFrameIndex() const
{
    return FrameIndex;
}

class FVkShader;

struct FScriptableRenderPipeline
{
    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Bind() = 0;
    
    TUniquePtr<FVkShader>                       VertexShader; // Replace with a TMap<EShaderStage, FVkShader>
    TUniquePtr<FVkShader>                       PixelShader;
    TUniquePtr<FVkPipeline>                     Pipeline;
    TUniquePtr<FVkPipelineLayout>               PipelineLayout;
};

struct FOpaqueRenderPipeline : public FScriptableRenderPipeline
{
    virtual void Initialize() override;
    virtual void Shutdown() override;
    virtual void Bind() override;
};

class FVulkanForwardRenderer : public FVulkanRenderer
{
    TMap<FString, FScriptableRenderPipeline*>     ScriptableRenderPipeline;

    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void Bind()override;
};