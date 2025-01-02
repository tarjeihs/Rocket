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
    
    virtual void Init();
    virtual void Shutdown();
    void Render();
    virtual void Resize();

    virtual void Bind() = 0;
    virtual void BindImGui() = 0;

    inline FVkImage* GetColorAttachment16() const;
    inline FVkImage* GetDepthAttachmentD32() const;
    inline PVulkanCommandPool* GetCommandPool() const;
    inline PVulkanCommandBuffer* GetCommandBuffer() const;
    inline PVulkanSwapchain* GetSwapchain() const;
    inline SizeType GetFrameIndex() const;
    inline uint32 GetNextImageIndex() const;

    inline FVkMeshAllocator* GetMeshAllocator() const;

    void ImmediateSubmit(std::function<void(PVulkanCommandBuffer*)>&& Func);

protected:
    void BeginFrame();
    void EndFrame();

public: 
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
    TUniquePtr<FVkOverlay>                                ImGui;
        
    TUniquePtr<FVkImage>                                ColorAttachment16;
    TUniquePtr<FVkImage>                                ColorAttachment8;
    TUniquePtr<FVkImage>                                DepthAttachmentD32;
        
    SizeType                                            FrameIndex = 0;

    FVkPipelineLayout*                                  PipelineLayout;

    TArray<FVkDescriptorPool*>                          DescriptorPoolData;
    TArray<FVkDescriptorSet*>                           DescriptorSetData;
    TArray<FVkDescriptorSetLayout*>                     DescriptorSetLayoutData;
    
    TArray<FVkScriptableRendererPipeline*>              ScriptableRendererPipelineData;

    TUniquePtr<FVkMeshAllocator>                        MeshAllocator;
};

inline FVkImage* FVkRenderer::GetColorAttachment16() const
{
    return ColorAttachment16.Get();
}

inline FVkImage* FVkRenderer::GetDepthAttachmentD32() const
{
    return DepthAttachmentD32.Get();
}

inline PVulkanCommandPool* FVkRenderer::GetCommandPool() const
{
    return CommandPool[FrameIndex].Get();
}

inline PVulkanCommandBuffer* FVkRenderer::GetCommandBuffer() const
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

inline FVkMeshAllocator* FVkRenderer::GetMeshAllocator() const
{
    return MeshAllocator.Get();
}

inline uint32 FVkRenderer::GetNextImageIndex() const
{
    return NextImageIndex[FrameIndex];
}