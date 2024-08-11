#include "EnginePCH.h"
#include "WindowsWindow.h"

#include <glfw/glfw3.h>

#include "Core/Assert.h"

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
