#pragma once

struct IComponent {};

struct FUUIDComponent : IComponent
{
    FUUIDComponent() = default;

    uint64_t UUID;
};

struct FTagComponent : IComponent
{
    FTagComponent() = default;
    FTagComponent(const std::string& InTag) : Tag(InTag) {}

    std::string Tag;
};

struct FCameraComponent : IComponent
{
    FCameraComponent() = default;
    FCameraComponent(ICamera* InCamera) : Camera(InCamera) {}

    ICamera* Camera;
};
