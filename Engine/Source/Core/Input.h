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
    inline static bool IsKeyPressed(int KeyCode)
    {
        return GInput->IsKeyPressedImpl(KeyCode);
    }

    inline static bool IsKeyHold(int32_t KeyCode, float Duration)
    {
        return GInput->IsKeyHoldImpl(KeyCode, Duration);
    }

    inline static bool IsMouseButtonPressed(int32_t KeyCode)
    {
        return GInput->IsMouseButtonPressedImpl(KeyCode);
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

    inline static SKeyPressData& GetKeyPressData(int32_t KeyCode)
    {
        return GInput->KeyPressData[KeyCode];
    }
    
protected:
    virtual bool IsKeyPressedImpl(int32_t KeyCode) = 0;
    virtual bool IsKeyHoldImpl(int32_t KeyCode, float Duration) = 0;
    virtual bool IsMouseButtonPressedImpl(int32_t KeyCode) = 0;

    virtual float GetMouseXImpl() = 0;
    virtual float GetMouseYImpl() = 0;
    
    virtual std::pair<float, float> GetMousePositionImpl() = 0;

protected:
    std::unordered_map<int32_t, SKeyPressData> KeyPressData;
    
private:
    static PInput* GInput;
};