#include "EnginePCH.h"
#include "AssetManager.h"

#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Vulkan/VulkanMesh.h"

void SAssetTable::AddAsset(SAsset&& Asset)
{
    Assets.push_back(std::move(Asset));
}

void SAssetTable::AddAsset(const SAsset& Asset)
{
    Assets.push_back(Asset);
}

const SAsset* SAssetTable::GetAssetByUUID(const uint32_t UUID) const
{
    auto It = std::find_if(Assets.begin(), Assets.end(), [UUID](const SAsset& Asset)
    {
        return Asset.UUID == UUID;
    });

    if (It != Assets.end()) 
    {
        return &(*It);
    }
    return nullptr;
}

const SAsset* SAssetTable::GetAssetByName(const std::string& Name) const
{
    uint32_t Hash = FNV1aHash(Name);
    auto It = std::find_if(Assets.begin(), Assets.end(), [Hash](const SAsset& Asset)
    {
        return Asset.UUID == Hash;
    });

    if (It != Assets.end()) 
    {
        return &(*It);
    }
    return nullptr;
}

void PAssetManager::LoadAsset(const std::string& FilePath, const std::string& Type, const std::string& Name)
{
    SBlob Blob = PFileSystem::ReadFileBinary(FilePath);
    SAsset Asset;
    SAssetTable& Table = Tables[Type];

    if (Type == "Mesh")
    {
        Asset.Object = new PVulkanMesh();
    }
    else if (Type == "Material")
    {
        Asset.Object = new PVulkanMaterial();
    }

    Asset.UUID = FNV1aHash(Name);
    Asset.Object->Deserialize(Blob); 
    Table.AddAsset(Asset);
}

const SAsset* PAssetManager::GetAsset(const std::string& Type, const std::string& Name) const 
{
    const SAssetTable& Table = Tables.at(Type);
    const SAsset* Asset = Table.GetAssetByName(Name);
    return Asset;
}

void PAssetManager::Dispose() 
{
    for (auto It = BeginTableIterator(); It != EndTableIterator(); ++It)
    {
        for (const auto& Asset : It->second)
        {
            Asset.Object->Cleanup();
        }
    }
}