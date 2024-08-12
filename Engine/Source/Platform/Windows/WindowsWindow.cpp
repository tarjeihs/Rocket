#include "EnginePCH.h"
#include "WindowsWindow.h"

#include <glfw/glfw3.h>

#include "Core/Assert.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanSwapchain.h"

void PWindowsWindow::CreateNativeWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    NativeWindow = glfwCreateWindow(WindowSpecification.Width, WindowSpecification.Height, WindowSpecification.Name, nullptr, nullptr);
    RK_ENGINE_ASSERT(NativeWindow, "Failed to create GLFW window");

    glfwSetWindowUserPointer((GLFWwindow*)NativeWindow, &WindowUserData);
    glfwSetInputMode((GLFWwindow*)NativeWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwMakeContextCurrent((GLFWwindow*)NativeWindow);
    glfwSwapInterval(1);

	glfwSetFramebufferSizeCallback((GLFWwindow*)NativeWindow, [](GLFWwindow* GlfwWindow, int Width, int Height)
	{
		IWindow* Window = GetWindow();

		// Handle edgecase where framebuffer size is 0 (minimized)
		if (Width == 0 || Height == 0)
		{
			if (!Window->IsMinimized()) Window->SetIsMinimized(true);
		}
		else
		{
			if (Window->IsMinimized()) Window->SetIsMinimized(false);
		}

		PVulkanRenderer* Renderer = GetRenderer<PVulkanRenderer>();
		Renderer->GetSwapchain()->RegenerateSwapchain(Renderer->GetInstance().SurfaceInterface, Renderer->GetDevice().PhysicalDevice, Renderer->GetDevice().LogicalDevice, Renderer->GetAllocator());
	});
}

void PWindowsWindow::DestroyNativeWindow()
{
    glfwDestroyWindow((GLFWwindow*)NativeWindow);
    glfwTerminate();
}

void PWindowsWindow::Poll()
{
    glfwPollEvents();
}

void PWindowsWindow::Swap()
{
    glfwSwapBuffers((GLFWwindow*)NativeWindow);
}

bool PWindowsWindow::ShouldClose() const
{
    return glfwWindowShouldClose((GLFWwindow*)NativeWindow);
}

bool PWindowsWindow::IsMinimized() const
{
    return bIsMinimized;
}

void PWindowsWindow::SetIsMinimized(bool bMinimized)
{
	bIsMinimized = bMinimized;
}
