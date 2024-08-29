#pragma once

#include "Core/Input.h"

class PGenericInput : public PInput 
{
protected:
    virtual bool GetKeyPressedImpl(int32_t KeyCode) override;
    virtual bool GetMouseButtonPressedImpl(int32_t KeyCode) override;

    virtual float GetMouseXImpl() override;
    virtual float GetMouseYImpl() override;

    virtual std::pair<float, float> GetMousePositionImpl() override;

    virtual int32_t GetFirstJoystickIDImpl() const override;
    virtual bool GetJoystickButtonImpl(const int32_t JoystickID, const int32_t Button) override;
    virtual float GetJoystickAxisImpl(const int32_t JoystickID, const int32_t Axis) override;
};