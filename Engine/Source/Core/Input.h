#pragma once

#include <chrono>
#include <unordered_map>
#include <utility>

struct SKeyPressData
{
    std::chrono::steady_clock::time_point Start;

    bool bIsPressed;
};

class PInput
{
public:
    virtual ~PInput() = default;

    inline static bool GetKeyPressed(int KeyCode)
    {
        return GInput->GetKeyPressedImpl(KeyCode);
    }

    inline static bool GetKeyHold(int32_t KeyCode, float Duration)
    {
        return GInput->GetKeyHoldImpl(KeyCode, Duration);
    }

    inline static bool GetMouseButtonPressed(int32_t KeyCode)
    {
        return GInput->GetMouseButtonPressedImpl(KeyCode);
    }

    inline static float GetMouseX()
    {
        return GInput->GetMouseXImpl();
    }

    inline static float GetMouseY()
    {
        return GInput->GetMouseYImpl();
    }

    inline static std::pair<float, float> GetMousePosition()
    {
        return GInput->GetMousePositionImpl();
    }

    inline static int32_t GetFirstJoystickID()
    {
        return GInput->GetFirstJoystickIDImpl();
    }

    inline static bool GetJoystickButton(const int32_t JoystickID, const int32_t Button)
    {
        return GInput->GetJoystickButtonImpl(JoystickID, Button);
    }

    inline static float GetJoystickAxis(const int32_t JoystickID, const int32_t Axis)
    {
        return GInput->GetJoystickAxisImpl(JoystickID, Axis);
    }

    inline static SKeyPressData& GetKeyPressData(int32_t KeyCode)
    {
        return GInput->KeyPressData[KeyCode];
    }

protected:
    virtual bool GetKeyPressedImpl(int32_t KeyCode) = 0;
    virtual bool GetKeyHoldImpl(int32_t KeyCode, float Duration) = 0;
    virtual bool GetMouseButtonPressedImpl(int32_t KeyCode) = 0;

    virtual float GetMouseXImpl() = 0;
    virtual float GetMouseYImpl() = 0;
    
    virtual std::pair<float, float> GetMousePositionImpl() = 0;

    virtual int32_t GetFirstJoystickIDImpl() const = 0;
    virtual bool GetJoystickButtonImpl(const int32_t JoystickID, const int32_t Button) = 0;
    virtual float GetJoystickAxisImpl(const int32_t JoystickID, const int32_t Axis) = 0;

protected:
    std::unordered_map<int32_t, SKeyPressData> KeyPressData;
    
private:
    static PInput* GInput;
};