#pragma once

#include "EngineTypes.h"
#include "Renderer/Common/Renderer.h"
#include "Renderer/Settings.h"
#include "Types/UniquePtr.h"

class IRenderer;
class PVulkanAllocator;
class PVulkanSwapchain;
class PVulkanCommandPool;
class PVulkanCommandBuffer;
class PVulkanRenderGraph;
class PVulkanImage;
class FVkBuffer;
class FVkPipeline;
class FVkPipelineLayout;
class FVkDescriptorPool;
class FVkDescriptorSet;
class FVkDescriptorSetLayout;
class FVkScriptableRendererPipeline;
class FVkSceneBuffer;

class FVkRenderer : public IRenderer
{
public:
    using Super = FVkRenderer;

    virtual void Init();
    virtual void Shutdown();
    void Render();
    void Resize();

    virtual void Bind() = 0;

    inline PVulkanImage* GetColorAttachmentImage() const;
    inline PVulkanImage* GetDepthAttachmentImage() const;
    inline PVulkanCommandPool* GetCommandPool() const;
    inline PVulkanCommandBuffer* GetCommandBuffer() const;
    inline PVulkanRenderGraph* GetRenderGraph() const;
    inline PVulkanSwapchain* GetSwapchain() const;
    inline SizeType GetFrameIndex() const;

    inline FVkSceneBuffer* GetSceneBuffer() const;

    void ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func);

protected:
    void BeginFrame();
    void EndFrame();

protected: 
    TUniquePtr<PVulkanCommandPool>                      CommandPool                     [CONCURRENT_FRAME_COUNT];
    TUniquePtr<PVulkanCommandBuffer>                    CommandBuffer                   [CONCURRENT_FRAME_COUNT];
    VkSemaphore                                         SwapchainSemaphore              [CONCURRENT_FRAME_COUNT];
    VkSemaphore                                         RenderSemaphore                 [CONCURRENT_FRAME_COUNT];
    VkFence                                             RenderFence                     [CONCURRENT_FRAME_COUNT];
    uint32                                              NextImageIndex                  [CONCURRENT_FRAME_COUNT];
        
    TUniquePtr<PVulkanCommandBuffer>                    ImmediateCommandBuffer;
    TUniquePtr<PVulkanCommandPool>                      ImmediateCommandPool;
    VkFence                                             ImmediateRenderFence;
        
    TUniquePtr<PVulkanSwapchain>                        Swapchain;
    TUniquePtr<PVulkanRenderGraph>                      RenderGraph;
        
    TUniquePtr<PVulkanImage>                                ColorAttachmentImage;
    TUniquePtr<PVulkanImage>                                DepthAttachmentImage;
        
    SizeType                                            FrameIndex = 0;

    FVkPipelineLayout*                                  SharedPipelineLayout;

    TArray<FVkDescriptorPool*>                          DescriptorPoolData;
    TArray<FVkDescriptorSet*>                           DescriptorSetData;
    TArray<FVkDescriptorSetLayout*>                     DescriptorSetLayoutData;
    
    TMap<FString, FVkScriptableRendererPipeline*>       ScriptableRendererPipelineData;

    TUniquePtr<FVkSceneBuffer>                           SceneBuffer;
};

inline PVulkanImage* FVkRenderer::GetColorAttachmentImage() const
{
    return ColorAttachmentImage.Get();
}

inline PVulkanImage* FVkRenderer::GetDepthAttachmentImage() const
{
    return DepthAttachmentImage.Get();
}

inline PVulkanCommandPool* FVkRenderer::GetCommandPool() const
{
    return CommandPool[FrameIndex].Get();
}

inline PVulkanCommandBuffer* FVkRenderer::GetCommandBuffer() const
{
    return CommandBuffer[FrameIndex].Get();
}

inline PVulkanRenderGraph* FVkRenderer::GetRenderGraph() const
{
    return RenderGraph.Get();
}

inline PVulkanSwapchain* FVkRenderer::GetSwapchain() const
{
    return Swapchain.Get();
}

inline SizeType FVkRenderer::GetFrameIndex() const
{
    return FrameIndex;
}

inline FVkSceneBuffer* FVkRenderer::GetSceneBuffer() const
{
    return SceneBuffer.Get();
}