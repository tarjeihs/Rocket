#include "EnginePCH.h"
#include "Registry.h"

SEntity PRegistry::CreateEntity()
{
    SEntity Entity{};
    Entity.EntityID = Registry.create();
    return Entity;
}

void PRegistry::DestroyEntity(SEntityID EntityID)
{
    Registry.destroy(EntityID);
}

bool PRegistry::IsValid(SEntityID EntityID) const
{
    return Registry.valid(EntityID);
}
