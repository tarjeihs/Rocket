#include "EnginePCH.h"
#include "VulkanOverlay.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Core/Window.h"
#include "Core/Engine.h"
#include "Renderer/RHI.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanDescriptor.h"
#include "Renderer/Vulkan/VulkanInstance.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanRenderGraph.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanImage.h"

void PVulkanOverlay::Init()
{
	std::vector<SVulkanDescriptorPoolRatio> Sizes = {
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
	};

	DescriptorPool = new PVulkanDescriptorPool();
	DescriptorPool->CreatePool(1000, Sizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

	VkFormat ColorAttachmentFormatPointer = GetRHI()->GetSceneRenderer()->GetSwapchain()->GetSurfaceFormat().format;

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)GetWindow()->GetNativeWindow(), true);

	ImGui_ImplVulkan_InitInfo ImGuiInitInfo{};
	ImGuiInitInfo.Instance = GetRHI()->GetInstance()->GetVkInstance();
	ImGuiInitInfo.PhysicalDevice = GetRHI()->GetDevice()->GetVkPhysicalDevice();
	ImGuiInitInfo.Device = GetRHI()->GetDevice()->GetVkDevice();
	ImGuiInitInfo.Queue = GetRHI()->GetDevice()->GetGraphicsQueue();
	ImGuiInitInfo.DescriptorPool = DescriptorPool->GetVkDescriptorPool();
	ImGuiInitInfo.MinImageCount = 3;
	ImGuiInitInfo.ImageCount = 3;
	ImGuiInitInfo.UseDynamicRendering = true;
	ImGuiInitInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	ImGuiInitInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	ImGuiInitInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &ColorAttachmentFormatPointer;
	ImGuiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&ImGuiInitInfo);
	ImGui_ImplVulkan_CreateFontsTexture();

	GetRHI()->GetSceneRenderer()->GetOverlayRenderGraph()->AddCommand([&](PVulkanFrame* Frame) 
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Metrics");
		ImGui::Text("Frame Rate: %f", 1.0f / GetEngine()->Timestep.GetDeltaTime());
		ImGui::Text("Frame Time: %fms", GetEngine()->Timestep.GetDeltaTime());
		ImGui::Text("Engine Time: %fs", GetEngine()->Timestep.GetElapsedTime());
		ImGui::End();

		OnRender.Broadcast();

		ImGui::Render();

		PVulkanImage* Image = GetRHI()->GetSceneRenderer()->GetSwapchain()->GetSwapchainImages()[Frame->TransientFrameData.NextImageIndex];

		VkRenderingAttachmentInfo ColorRenderingAttachmentAttachment{};
		ColorRenderingAttachmentAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		ColorRenderingAttachmentAttachment.pNext = nullptr;
		ColorRenderingAttachmentAttachment.imageView = Image->GetVkImageView();
		ColorRenderingAttachmentAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		ColorRenderingAttachmentAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		ColorRenderingAttachmentAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfo RenderingInfo = {};
		RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		RenderingInfo.pNext = nullptr;
		RenderingInfo.flags = 0;
		RenderingInfo.renderArea.offset = { 0, 0 };
		RenderingInfo.renderArea.extent = GetRHI()->GetSceneRenderer()->GetSwapchain()->GetVkExtent();
		RenderingInfo.layerCount = 1;
		RenderingInfo.viewMask = 0;
		RenderingInfo.colorAttachmentCount = 1;
		RenderingInfo.pColorAttachments = &ColorRenderingAttachmentAttachment;
		RenderingInfo.pDepthAttachment = nullptr;
		RenderingInfo.pStencilAttachment = nullptr;
		RenderingInfo.pNext = nullptr;

		vkCmdBeginRendering(Frame->GetCommandBuffer()->GetVkCommandBuffer(), &RenderingInfo);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Frame->GetCommandBuffer()->GetVkCommandBuffer());
		vkCmdEndRendering(Frame->GetCommandBuffer()->GetVkCommandBuffer());	
	});
}

void PVulkanOverlay::Shutdown()
{
	ImGui_ImplVulkan_Shutdown();
	
	DescriptorPool->DestroyPool();
	delete DescriptorPool;

	OnRender.Clear();
}