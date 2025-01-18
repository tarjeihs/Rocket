#pragma once

#include "Core/Delegate.h"
#include "EngineTypes.h"
#include "Renderer/Common/Renderer.h"
#include "Renderer/Settings.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Types/String.h"
#include "Types/UniquePtr.h"
#include "Renderer/Allocators/VkMeshAllocator.h"
#include "VulkanTexture2D.h"

class FVkOverlay;
class IRenderer;
class FVkAllocator;
class PVulkanSwapchain;
class FVkCommandPool;
class FVkCommandBuffer;
class PVulkanRenderGraph;
class PVulkanImage;
class FVkBuffer;
class FVkPipeline;
class FVkPipelineLayout;
class FVkDescriptorPool;
class FVkDescriptorSet;
class FVkDescriptorSetLayout;
class IPipeline2;
class FVkSceneInstanceManager;

class FVkRenderer : public IRenderer
{
public:
    using Super = FVkRenderer;

    TDelegate<> OnSubmit;

    FVkDescriptorSetLayout* 	DescriptorSetLayout     = nullptr;
    FVkDescriptorSet* 			DescriptorSet           = nullptr;
    FVkDescriptorPool* 			DescriptorPool          = nullptr;
    TMap<FString, FVkBuffer**>  Buffers;
    TMap<FString, FVkImage**>   RWTexture2D;
    TArray<FVkTexture2D*>           Texture2Ds;
    
    virtual void Init();
    virtual void Shutdown();
    void Render();
    virtual void Resize();

    virtual void Bind() = 0;
    virtual void BindImGui() = 0;

    inline FVkImage* GetColorAttachment16() const;
    inline FVkImage* GetDepthAttachmentD32() const;
    inline FVkCommandPool* GetCommandPool() const;
    inline FVkCommandBuffer* GetCommandBuffer() const;
    inline PVulkanSwapchain* GetSwapchain() const;
    inline SizeType GetFrameIndex() const;
    inline uint32 GetNextImageIndex() const;

    inline FVkStaticMeshBuffer* GetMeshAllocator() const;

    void ImmediateSubmit(std::function<void(FVkCommandBuffer*)>&& Func);

protected:
    void BeginFrame();
    void EndFrame();

public: 
    TUniquePtr<FVkCommandPool>                      CommandPool                     [CONCURRENT_FRAME_COUNT];
    TUniquePtr<FVkCommandBuffer>                    CommandBuffer                   [CONCURRENT_FRAME_COUNT];
    VkSemaphore                                         SwapchainSemaphore              [CONCURRENT_FRAME_COUNT];
    VkSemaphore                                         RenderSemaphore                 [CONCURRENT_FRAME_COUNT];
    VkFence                                             RenderFence                     [CONCURRENT_FRAME_COUNT];
    uint32                                              NextImageIndex                  [CONCURRENT_FRAME_COUNT];
        
    TUniquePtr<FVkCommandBuffer>                    ImmediateCommandBuffer;
    TUniquePtr<FVkCommandPool>                      ImmediateCommandPool;
    VkFence                                             ImmediateRenderFence;
        
    TUniquePtr<PVulkanSwapchain>                        Swapchain;
    TUniquePtr<FVkOverlay>                               ImGui;
        
    SizeType                                            FrameIndex = 0;

    class IRendererFrontend*                            RendererFrontend;








    TUniquePtr<FVkImage>                                IntermediateColorAttachment;
    TUniquePtr<FVkImage>                                PresentColorAttachment;
    TUniquePtr<FVkImage>                                DepthAttachmentD32;
    FVkPipelineLayout*                                  PipelineLayout;
    TArray<IPipeline2*>              ScriptableRendererPipelineData;
    TUniquePtr<FVkStaticMeshBuffer>                     MeshAllocator;
};

inline FVkImage* FVkRenderer::GetColorAttachment16() const
{
    return IntermediateColorAttachment.Get();
}

inline FVkImage* FVkRenderer::GetDepthAttachmentD32() const
{
    return DepthAttachmentD32.Get();
}

inline FVkCommandPool* FVkRenderer::GetCommandPool() const
{
    return CommandPool[FrameIndex].Get();
}

inline FVkCommandBuffer* FVkRenderer::GetCommandBuffer() const
{
    return CommandBuffer[FrameIndex].Get();
}

inline PVulkanSwapchain* FVkRenderer::GetSwapchain() const
{
    return Swapchain.Get();
}

inline SizeType FVkRenderer::GetFrameIndex() const
{
    return FrameIndex;
}

inline FVkStaticMeshBuffer* FVkRenderer::GetMeshAllocator() const
{
    return MeshAllocator.Get();
}

inline uint32 FVkRenderer::GetNextImageIndex() const
{
    return NextImageIndex[FrameIndex];
}