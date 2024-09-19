#pragma once

#include "Utils/FileSystem.h"

struct IAssetMarshalInterface
{
    virtual ~IAssetMarshalInterface() = default;

    virtual void Init() {}
    virtual void Cleanup() {}

    virtual void Serialize(SBlob& Blob) {}
    virtual void Deserialize(SBlob& Blob) {}
};

struct SAsset
{
    template<typename TClass>
    TClass* GetObject() const;

    IAssetMarshalInterface* Object;

    uint32_t UUID;

    friend class SAssetTable;
    friend class PAssetManager;
};

template<typename TClass>
inline TClass* SAsset::GetObject() const
{
    return static_cast<TClass*>(Object);
}