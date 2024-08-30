#pragma once

#include "Math/Transform.h"
#include "Utils/UUID64.h"

struct IComponent {};

struct STagComponent : IComponent
{
    std::string Tag;
};

struct STransformComponent : IComponent
{
    STransform Transform;
};

struct SUUIDComponent : IComponent
{
    SUUID UUID;
};