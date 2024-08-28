#include "GenericInput.h"

#include <GLFW/glfw3.h>

#include "Core/Engine.h"
#include "Core/KeyCode.h"
#include "Core/Window.h"

#if defined(RK_PLATFORM_WINDOWS) || defined(RK_PLATFORM_LINUX)
PInput* PInput::GInput = new PGenericInput();
#endif

bool PGenericInput::GetKeyPressedImpl(int32_t KeyCode)
{
    auto Window = static_cast<GLFWwindow*>(GetWindow()->GetNativeWindow());
    auto State = glfwGetKey(Window, KeyCode);

    return State == GLFW_PRESS || State == GLFW_REPEAT;
}

bool PGenericInput::GetKeyHoldImpl(int32_t KeyCode, float Threshold)
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

bool PGenericInput::GetMouseButtonPressedImpl(int32_t KeyCode)
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

int32_t PGenericInput::GetFirstJoystickIDImpl() const
{
    for (int32_t JoystickID = RK_JOYSTICK_DEVICE_1; JoystickID <= RK_JOYSTICK_DEVICE_LAST; ++JoystickID)
    {
        if (glfwJoystickPresent(JoystickID))
        {
            return JoystickID;
        }
    }
    return -1;
}

bool PGenericInput::GetJoystickButtonImpl(const int32_t JoystickID, const int32_t Button)
{
    int32_t ButtonCount;
    const unsigned char* Buttons = glfwGetJoystickButtons(JoystickID, &ButtonCount);
    if (Button < ButtonCount)
    {
        return Buttons[Button] == GLFW_PRESS;
    }
    return false;
}

float PGenericInput::GetJoystickAxisImpl(const int32_t JoystickID, const int32_t Axis)
{
    int32_t AxisCount;
    const float* Axes = glfwGetJoystickAxes(JoystickID, &AxisCount);
    const float Value = Axes[Axis];
    return Value;
}