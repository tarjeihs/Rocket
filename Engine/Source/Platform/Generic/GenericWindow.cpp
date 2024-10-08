#include "EnginePCH.h"
#include "GenericWindow.h"

#include <GLFW/glfw3.h>

#include "Core/Assert.h"
#include "Core/Input.h"
#include "Core/Camera.h"
#include "Scene/Scene.h"
#include "Renderer/VulkanRHI.h"

void PGenericWindow::CreateNativeWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    NativeWindow = glfwCreateWindow(WindowSpecification.Width, WindowSpecification.Height, WindowSpecification.Name, nullptr, nullptr);
    RK_ASSERT(NativeWindow, "Failed to create GLFW window");

    glfwMakeContextCurrent((GLFWwindow*)NativeWindow);

	glfwSetKeyCallback((GLFWwindow*)NativeWindow, [](GLFWwindow* Window, int32_t KeyCode, int32_t ScanCode, int32_t Action, int32_t Mod)
	{
		if (KeyCode == RK_KEY_ESCAPE)
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

		GetScene()->GetCamera()->ApplySettings();
		GetRHI()->Resize();
	});

	glfwSetWindowFocusCallback((GLFWwindow*)NativeWindow, [](GLFWwindow* Window, int32_t Focused)
	{
		if (Focused)
		{
			GetWindow()->OnWindowFocusDelegate.Broadcast(Focused);

			GetWindow()->SetIsFocused(true);
		}
		else 
		{
			GetWindow()->OnWindowFocusDelegate.Broadcast(Focused);

			GetWindow()->SetIsFocused(false);
		}
	});

	glfwSetWindowPosCallback((GLFWwindow*)NativeWindow, [](GLFWwindow* Window, int32_t PositionX, int32_t PositionY)
	{
		GetWindow()->GetWindowSpecification().PositionX = PositionX;
		GetWindow()->GetWindowSpecification().PositionY = PositionY;
	});
}

void PGenericWindow::DestroyNativeWindow()
{
    glfwDestroyWindow((GLFWwindow*)NativeWindow);
    glfwTerminate();
}

void PGenericWindow::Poll()
{
    glfwPollEvents();
}

bool PGenericWindow::ShouldClose() const
{
    return glfwWindowShouldClose((GLFWwindow*)NativeWindow);
}

bool PGenericWindow::IsMinimized() const
{
    return bIsMinimized;
}

void PGenericWindow::SetIsMinimized(bool bMinimized)
{
	bIsMinimized = bMinimized;
}

void PGenericWindow::SetIsFocused(bool bFocused)
{
	bIsFocused = bFocused;
}

void PGenericWindow::WaitEventOrTimeout(float TimeoutSeconds)
{
	glfwWaitEventsTimeout(TimeoutSeconds);
}

bool PGenericWindow::IsFocused() const
{
    return bIsFocused;
}
