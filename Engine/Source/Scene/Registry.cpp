#include "EnginePCH.h"
#include "Registry.h"

FEntity PRegistry::CreateEntity()
{
    FEntity Entity{};
    Entity.EntityID = Registry.create();
    return Entity;
}

void PRegistry::DestroyEntity(FEntityID EntityID)
{
    Registry.destroy(EntityID);
}

bool PRegistry::IsValid(FEntityID EntityID) const
{
    return Registry.valid(EntityID);
}
