#pragma once

#include "Core/Input.h"

class PGenericInput : public PInput 
{
protected:
    virtual bool IsKeyPressedImpl(int32_t KeyCode) override;
    virtual bool IsKeyHoldImpl(int32_t KeyCode, float Threshold) override;
    virtual bool IsMouseButtonPressedImpl(int32_t KeyCode) override;

    virtual float GetMouseXImpl() override;
    virtual float GetMouseYImpl() override;

    virtual std::pair<float, float> GetMousePositionImpl() override;
};