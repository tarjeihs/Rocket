#include "GenericInput.h"

#include <GLFW/glfw3.h>

#include "Core/Engine.h"
#include "Core/Window.h"

#if defined(RK_PLATFORM_WINDOWS) || defined(RK_PLATFORM_LINUX)
PInput* PInput::GInput = new PGenericInput();
#endif

bool PGenericInput::IsKeyPressedImpl(int32_t KeyCode)
{
    auto Window = static_cast<GLFWwindow*>(GetWindow()->GetNativeWindow());
    auto State = glfwGetKey(Window, KeyCode);

    return State == GLFW_PRESS || State == GLFW_REPEAT;
}

bool PGenericInput::IsKeyHoldImpl(int32_t KeyCode, float Threshold)
{
    auto Now = std::chrono::steady_clock::now();

    if (KeyPressData[KeyCode].bIsPressed)
    {
        auto Duration = std::chrono::duration<double>(Now - KeyPressData[KeyCode].Start).count();
        if (Duration >= Threshold)
        {
            KeyPressData[KeyCode].bIsPressed = false;
            return true;
        }
    }
    return false;
}

bool PGenericInput::IsMouseButtonPressedImpl(int32_t KeyCode)
{
    auto Window = static_cast<GLFWwindow*>(GetWindow()->GetNativeWindow());
    auto State = glfwGetMouseButton(Window, KeyCode);

    return State == GLFW_PRESS;
}

float PGenericInput::GetMouseXImpl()
{
    auto[MouseX, MouseY] = GetMousePositionImpl();
    return MouseX;
}

float PGenericInput::GetMouseYImpl()
{
    auto[MouseX, MouseY] = GetMousePositionImpl();
    return MouseY;
}

std::pair<float, float> PGenericInput::GetMousePositionImpl()
{
    auto Window = static_cast<GLFWwindow*>(GetWindow()->GetNativeWindow());
    double MouseX, MouseY;
    glfwGetCursorPos(Window, &MouseX, &MouseY);
    return { (float)MouseX, (float)MouseY };
}