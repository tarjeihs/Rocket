#include "EnginePCH.h"
#include "VkOverlay.h"

#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanInstance.h"
#include "Renderer/Vulkan/VkRenderer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"

void FVkOverlay::Init()
{
    FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = 
    {
        {
		    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        },
        1000,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
    };

	Info.DescriptorPool = new FVkDescriptorPool();
	Info.DescriptorPool->Initialize(DescriptorPoolCreateInfo);

	ImGui_ImplVulkan_InitInfo ImGuiInitInfo = {};
	ImGuiInitInfo.Instance = GetRHI()->GetInstance()->GetVkInstance();
	ImGuiInitInfo.PhysicalDevice = GetRHI()->GetDevice()->GetVkPhysicalDevice();
	ImGuiInitInfo.Device = GetRHI()->GetDevice()->GetVkDevice();
	ImGuiInitInfo.Queue = GetRHI()->GetDevice()->GetGraphicsQueue();
	ImGuiInitInfo.DescriptorPool = Info.DescriptorPool->Info.Handle;
	ImGuiInitInfo.MinImageCount = 3;
	ImGuiInitInfo.ImageCount = 3;
	ImGuiInitInfo.UseDynamicRendering = true;
	ImGuiInitInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	ImGuiInitInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	ImGuiInitInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainSurfaceFormat.format;
	ImGuiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)GetWindow()->GetNativeWindow(), true);
	ImGui_ImplVulkan_Init(&ImGuiInitInfo);
	ImGui_ImplVulkan_CreateFontsTexture();
}

void FVkOverlay::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();

    Info.DescriptorPool->Shutdown();
}

void FVkOverlay::Execute()
{
	VkRenderingAttachmentInfo ColorRenderingAttachmentAttachment = {};
	ColorRenderingAttachmentAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	ColorRenderingAttachmentAttachment.pNext = nullptr;
	ColorRenderingAttachmentAttachment.imageView = GetRHI()->GetRenderer()->ColorAttachment8->Info.ImageViewHandle;
	ColorRenderingAttachmentAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	ColorRenderingAttachmentAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	ColorRenderingAttachmentAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	VkRenderingInfo RenderingInfo = {};
	RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	RenderingInfo.pNext = nullptr;
	RenderingInfo.flags = 0;
	RenderingInfo.renderArea.offset = { 0, 0 };
	RenderingInfo.renderArea.extent = GetRHI()->GetRenderer()->GetSwapchain()->Info.SwapchainImageExtent;
	RenderingInfo.layerCount = 1;
	RenderingInfo.viewMask = 0;
	RenderingInfo.colorAttachmentCount = 1;
	RenderingInfo.pColorAttachments = &ColorRenderingAttachmentAttachment;
	RenderingInfo.pDepthAttachment = nullptr;
	RenderingInfo.pStencilAttachment = nullptr;
	RenderingInfo.pNext = nullptr;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
//	ImGui::Begin("omegalul");
//	ImGui::Text("Current Frame Rate: %.1f FPS", 1.0f / GetEngine()->Timestep.GetDeltaTime());
//	ImGui::Text("Current Frame Time: %.3f ms", GetEngine()->Timestep.GetDeltaTime() * 1000.0f);
//	ImGui::Text("Engine Time: %.3fs", GetEngine()->Time.GetElapsedTimeAsSeconds());
//	ImGui::End();
    
    OnRender.Broadcast();

	ImGui::Render();

	vkCmdBeginRendering(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer(), &RenderingInfo);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer());
	vkCmdEndRendering(GetRHI()->GetRenderer()->GetCommandBuffer()->GetVkCommandBuffer());
}