#pragma once

#include "Scene/Scene.h"

class SEntity
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

    [[nodiscard]] SEntityID GetEntityID() const
    {
        return EntityID;
    }

private:
    SEntityID EntityID;

    friend class PRegistry;
};

template<typename TComponent, typename... TArgs>
TComponent& SEntity::AddComponent(TArgs&&... Args) const
{
    TComponent& Component = GetScene()->GetRegistry()->AddComponent<TComponent>(EntityID, std::forward<TArgs>(Args)...);
    return Component;
}

template<typename TComponent>
TComponent & SEntity::GetComponent() const
{
    TComponent& Component = GetScene()->GetRegistry()->GetComponent<TComponent>(EntityID);
    return Component;
}

template<typename TComponent>
bool SEntity::HasComponent() const
{
    return GetScene()->GetRegistry()->HasComponent<TComponent>(EntityID);
}

template<typename TComponent>
void SEntity::RemoveComponent()
{
    GetScene()->GetRegistry()->RemoveComponent<TComponent>(EntityID);
}

