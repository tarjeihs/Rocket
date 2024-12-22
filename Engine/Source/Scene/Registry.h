#pragma once

#include <entt/entt.hpp>

#include "Core/Assert.h"

using SRegistry = entt::registry;
using FEntityID = entt::entity;

struct FEntity;

class PRegistry
{
public:
    FEntity CreateEntity();
    void DestroyEntity(FEntityID EntityID);
    bool IsValid(FEntityID EntityID) const;

    template <typename TComponent, typename... TArgs>
    TComponent& AddComponent(FEntityID EntityID, TArgs... Args);

    template<typename TComponent>
    TComponent& GetComponent(FEntityID EntityID);

    template<typename... TComponent>
    bool HasComponent(FEntityID EntityID);

    template<typename TComponent>
    void RemoveComponent(FEntityID EntityID);

    template<typename... TComponents, typename TFunc>
    void View(TFunc&& Func);

private:
    SRegistry Registry;
};

template<typename TComponent, typename ... TArgs>
TComponent& PRegistry::AddComponent(FEntityID EntityID, TArgs... Args)
{
    RK_ASSERT(IsValid(EntityID), "Invalid EntityID.");
    return Registry.emplace<TComponent>(EntityID, std::forward<TArgs>(Args)...);
}

template<typename TComponent>
TComponent& PRegistry::GetComponent(FEntityID EntityID)
{
    RK_ASSERT(IsValid(EntityID), "Invalid EntityID.");
    return Registry.get<TComponent>(EntityID);
}

template<typename... TComponent>
bool PRegistry::HasComponent(FEntityID EntityID)
{
    RK_ASSERT(IsValid(EntityID), "Invalid EntityID.");
    return Registry.all_of<TComponent...>(EntityID);
}

template<typename TComponent>
void PRegistry::RemoveComponent(FEntityID EntityID)
{
    RK_ASSERT(IsValid(EntityID), "Invalid EntityID.");
    Registry.remove<TComponent>(EntityID);
}

template<typename ... TComponents, typename TFunc>
void PRegistry::View(TFunc&& Func)
{
    auto View = Registry.view<TComponents...>();
    View.each([&](TComponents&... Components)
    {
        Func(Components...);
    });
}