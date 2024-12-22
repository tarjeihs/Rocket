#pragma once

#include "Scene/Scene.h"

class FEntity
{
public:
    template<typename TComponent, typename... TArgs>
    TComponent& AddComponent(TArgs&&... Args) const;

    template<typename TComponent>
    TComponent& GetComponent() const;

    template<typename TComponent>
    bool HasComponent() const;

    template<typename TComponent>
    void RemoveComponent();

    [[nodiscard]] FEntityID GetEntityID() const
    {
        return EntityID;
    }

private:
    FEntityID EntityID;

    friend class PRegistry;
};

template<typename TComponent, typename... TArgs>
TComponent& FEntity::AddComponent(TArgs&&... Args) const
{
    TComponent& Component = GetScene()->GetRegistry()->AddComponent<TComponent>(EntityID, std::forward<TArgs>(Args)...);
    return Component;
}

template<typename TComponent>
TComponent & FEntity::GetComponent() const
{
    TComponent& Component = GetScene()->GetRegistry()->GetComponent<TComponent>(EntityID);
    return Component;
}

template<typename TComponent>
bool FEntity::HasComponent() const
{
    return GetScene()->GetRegistry()->HasComponent<TComponent>(EntityID);
}

template<typename TComponent>
void FEntity::RemoveComponent()
{
    GetScene()->GetRegistry()->RemoveComponent<TComponent>(EntityID);
}

