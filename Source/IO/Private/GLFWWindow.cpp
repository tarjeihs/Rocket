#include "RocketPCH.h"
#include "IO/Public/GLFWWindow.h"

#include "Core/Public/Assert.h"
#include "Core/Public/Camera.h"
#include "Core/Public/KeyCode.h"
#include "Scene/Public/Scene.h"

void CGLFWWindow::CreateNativeWindow()
{
	glfwSetErrorCallback([](int code, const char* desc)
	{
		fprintf(stderr, "GLFW error %d: %s\n", code, desc);
	});

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    NativeWindow = glfwCreateWindow(WindowSpecification.Width, WindowSpecification.Height, WindowSpecification.Name, nullptr, nullptr);
    RK_ASSERT(NativeWindow, "Failed to create GLFW window");

    //glfwMakeContextCurrent((GLFWwindow*)NativeWindow);

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
        //GetRHI()->Resize();
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

	GetWindow()->OnWindowFocusDelegate.Broadcast(1);
	GetWindow()->SetIsFocused(true);
}

void CGLFWWindow::DestroyNativeWindow()
{
    glfwDestroyWindow((GLFWwindow*)NativeWindow);
    glfwTerminate();
}

void CGLFWWindow::Poll()
{
    glfwPollEvents();
}

bool CGLFWWindow::ShouldClose() const
{
    return glfwWindowShouldClose((GLFWwindow*)NativeWindow);
}

bool CGLFWWindow::IsMinimized() const
{
    return bIsMinimized;
}

void CGLFWWindow::SetIsMinimized(bool bMinimized)
{
	bIsMinimized = bMinimized;
}

void CGLFWWindow::SetIsFocused(bool bFocused)
{
	bIsFocused = bFocused;
}

void CGLFWWindow::WaitEventOrTimeout(float TimeoutSeconds)
{
	glfwWaitEventsTimeout(TimeoutSeconds);
}

bool CGLFWWindow::IsFocused() const
{
    return bIsFocused;
}

void CGLFWWindow::SetFocus(bool bFocus)
{
	glfwFocusWindow((GLFWwindow*)NativeWindow);
}
