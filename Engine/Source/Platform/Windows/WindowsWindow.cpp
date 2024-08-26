#include "EnginePCH.h"
#include "WindowsWindow.h"

#include <GLFW/glfw3.h>

#include "Core/Assert.h"
#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Renderer/VulkanRHI.h"

void PWindowsWindow::CreateNativeWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    NativeWindow = glfwCreateWindow(WindowSpecification.Width, WindowSpecification.Height, WindowSpecification.Name, nullptr, nullptr);
    RK_ASSERT(NativeWindow, "Failed to create GLFW window");

    glfwSetWindowUserPointer((GLFWwindow*)NativeWindow, &WindowUserData);
    glfwMakeContextCurrent((GLFWwindow*)NativeWindow);

    glfwSetInputMode((GLFWwindow*)NativeWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glfwSetKeyCallback((GLFWwindow*)NativeWindow, [](GLFWwindow* Window, int32_t KeyCode, int32_t ScanCode, int32_t Action, int32_t Mod)
	{
		SWindowUserData& UserData = *(SWindowUserData*)glfwGetWindowUserPointer(Window);

		if (KeyCode == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(Window, true);
		}
	});

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

		Window->GetWindowSpecification().Width = Width;
		Window->GetWindowSpecification().Height = Height;

		while (GetWindow()->IsMinimized())
		{
			// Rough handling of window minimization.
			glfwWaitEvents();
		}

		GetScene()->ActiveCamera->SetPerspectiveProjection(glm::radians(66.0f), Window->GetAspectRatio(), 0.1f, 1000.f);
		GetRHI()->Resize();
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
