#pragma once

#include <entt/entt.hpp>

#include "Core/Assert.h"

using SRegistry = entt::registry;
using SEntityID = entt::entity;

struct SEntity;

class PRegistry
{
public:
    SEntity CreateEntity();
    void DestroyEntity(SEntityID EntityID);
    bool IsValid(SEntityID EntityID) const;

    template <typename TComponent, typename... TArgs>
    TComponent& AddComponent(SEntityID EntityID, TArgs... Args);

    template<typename TComponent>
    TComponent& GetComponent(SEntityID EntityID);

    template<typename... TComponent>
    bool HasComponent(SEntityID EntityID);

    template<typename TComponent>
    void RemoveComponent(SEntityID EntityID);

    template<typename... TComponents, typename TFunc>
    void View(TFunc&& Func);

private:
    SRegistry Registry;
};

template<typename TComponent, typename ... TArgs>
TComponent& PRegistry::AddComponent(SEntityID EntityID, TArgs... Args)
{
    RK_ASSERT(IsValid(EntityID), "Invalid EntityID.");
    return Registry.emplace<TComponent>(EntityID, std::forward<TArgs>(Args)...);
}

template<typename TComponent>
TComponent& PRegistry::GetComponent(SEntityID EntityID)
{
    RK_ASSERT(IsValid(EntityID), "Invalid EntityID.");
    return Registry.get<TComponent>(EntityID);
}

template<typename... TComponent>
bool PRegistry::HasComponent(SEntityID EntityID)
{
    RK_ASSERT(IsValid(EntityID), "Invalid EntityID.");
    return Registry.all_of<TComponent...>(EntityID);
}

template<typename TComponent>
void PRegistry::RemoveComponent(SEntityID EntityID)
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